# Fase 5 – Análisis y biblioteca (plan)

## Contexto

La fase 4 dejó la UI final con una biblioteca en memoria (`Importar` llena
un `std::vector<LibraryTrack>` con `shortcut:`). La fase 5 del spec pide:
**SQLite, importación de carpetas, BPM, beatgrid, tono y forma de onda.**
Requisitos del spec que entran aquí (sección Biblioteca):
- Importar carpetas con subcarpetas; título, artista y duración de tags.
- Análisis en segundo plano de BPM, beatgrid, tono y waveform → SQLite.
- Colecciones, búsqueda instantánea (título, artista, BPM, tono), orden por
  cualquier columna, filtro del mockup.
- Cargar en A/B con botón, doble clic o arrastrando.

## Decisiones

- **Análisis con qm-dsp** (Queen Mary DSP, GPLv2+, el de Mixxx; elegido
  por el usuario). Una sola dependencia, C++ sin dependencias externas
  (trae kissfft). ADR `docs/decisions/0002-analysis-library.md` (aubio vs
  Essentia vs qm-dsp, licencia, build en MSVC). Se actualizan SPEC.md
  (tabla del stack) y CLAUDE.md (Stack) en el mismo commit.
  - `cmake/Dependencies.cmake`: `FetchContent` a commit fijo de
    `github.com/c4dm/qm-dsp`; `cmake/QmDsp.cmake` declara
    `turbito_qmdsp` (STATIC) con solo los fuentes necesarios
    (onsets/DetectionFunction, tempotracking/TempoTrackV2, keydetection/
    GetKeyMode, chromagram, maths, base, dsp/transforms, ext/kissfft).
  - BPM/beatgrid: `DetectionFunction` (complex SD, hop 512 a la tasa
    original) → `TempoTrackV2::calculateBeatPeriod` + `calculateBeats`
    como Mixxx. Grid constante: BPM = mediana de los intervalos entre
    beats (redondeado a 0.01), primer beat = fase que mejor ajusta a los
    beats detectados, llevado a `[0, 60/BPM)`.
  - Tono: `GetKeyMode` sobre mono decimado; resultado → Camelot (función
    pura `toCamelot(keyIndex)`; La menor = 8A).
- **Tags con TagLib 2.x** (LGPL-2.1/MPL-1.1, CMake propio; necesita
  `utfcpp`, header-only, también por FetchContent). JUCE no lee ID3 ni
  Vorbis comments. Sin título → nombre de archivo; sin artista → "—".
  Duración desde `AudioFormatReader` (no de los tags).
- **SQLite amalgamation** (dominio público) por FetchContent con URL
  fija + `URL_HASH`; `turbito_sqlite` (STATIC, C). Una conexión, solo en
  el message thread; WAL. Ruta:
  `userApplicationDataDirectory/Turbito DJ/library.db`.
- **Esquema con migraciones por `PRAGMA user_version`**: lista ordenada
  de scripts SQL en `library/db/Schema.cpp`; v1:
  - `folders(id, path UNIQUE)`
  - `tracks(id, path UNIQUE, folder_id, title, artist, duration,
    bpm REAL NULL, first_beat REAL NULL, camelot TEXT NULL,
    waveform BLOB NULL, analyzed INTEGER)`
  - `history(track_id, loaded_at)`
  - Hot cues, playlists, placas y listas de Auto Mix llegan con su fase
    como nuevas migraciones.
- **Biblioteca en memoria, SQLite como persistencia**: al arrancar se
  cargan todas las pistas (sin el blob de waveform) a un vector; búsqueda,
  filtro y orden se hacen en memoria (5,000 filas: instantáneo y arranque
  < 3 s). Sin FTS.
- **Análisis en `juce::ThreadPool`** (hilos de baja prioridad,
  `max(1, cpus/2)`). Cada job lee el archivo por bloques con
  `AudioFormatReader` (sin decodificar entero: no 100 MB por pista), mezcla
  a mono y alimenta waveform, onsets y croma a la vez. El resultado vuelve
  por `MessageManager::callAsync` y el message thread lo escribe en SQLite.
  Nunca toca el hilo de audio.
- **Waveform**: pico absoluto por ventana de 50 ms (20 por segundo),
  `uint8` 0–255 → blob (~6 KB por 5 min). La UI muestra 160 barras = 8 s
  centrados en el playhead (scroll real); barras antes del inicio o tras
  el final = 0 (vacías, como en el mockup). Altura = pico · 48 px.
- **Beatgrid sin dibujar todavía**: el mockup no muestra líneas de beat en
  la waveform; dibujarlas es un cambio visual que se propone aparte. El
  grid se guarda y se usa en la fase 6 (sync) y 7 (cuantizar).
- **Importar** pide una carpeta (antes: archivos), la escanea con
  subcarpetas en un hilo (MP3, WAV, FLAC, AIFF/AIF), inserta las pistas y
  las encola para analizar. Reimportar una carpeta salta lo ya existente.
  **Analizar** encola las pistas sin analizar (p. ej. tras cerrar la app a
  medio análisis). La celda BPM/Tono muestra "—" hasta que hay resultado.
- **Colecciones**: Todas las pistas, Carpetas (cuenta de carpetas;
  elegirla muestra pistas de todas las carpetas, cada carpeta como hijo
  queda fuera), Historial (pistas cargadas en un deck, la más reciente
  arriba), Placas y drops (vacía hasta la fase 9). Playlists no está en el
  mockup: fase 10 (listas de Auto Mix).
- **Filtro del mockup**: Todas / BPM ±3 del deck A / Tono compatible con A
  (mismo número Camelot, ±1 con la misma letra, o mismo número con la
  otra letra) / Sin analizar. Funciones puras probadas.
- **Orden**: clic en cabecera ordena por esa columna (asc/desc), con
  `TableHeaderComponent` y `sortOrderChanged`.
- **Cargar arrastrando**: la tabla es fuente de drag
  (`getDragSourceDescription` → ruta), `AppView` es
  `DragAndDropContainer` y `DeckComponent` + su `WaveformView` son
  `DragAndDropTarget` y llaman a `onLoad(deck, file)`. Doble clic sigue
  cargando en A (como ahora).
- **Al cargar en un deck**: título/artista/BPM/tono del registro, waveform
  del blob, fila en `history`. BPM master de la barra = BPM del deck
  sonando (si suenan los dos, el que favorece el crossfader); "—" sin
  análisis. El BPM mostrado aún no sigue al pitch (fase 6).
- **Capas**: nuevo `src/library/` → `turbito_library` (depende de
  `turbito_core`, SQLite, TagLib, qm-dsp, `juce_audio_formats`). `ui` →
  `library` solo para el struct de pista y filtros; `app` conecta
  `Library` con `LibraryComponent` y el motor. La UI deja de tener la
  lista propia: `LibraryComponent::setTracks` sigue siendo la entrada y
  se borra `addFiles`/`importFiles` con su `shortcut:`.

## Diseño

```
app/MainComponent
  ├─ library::Library  (message thread)
  │    ├─ db/Database, db/Schema (migraciones), db/TrackRepository
  │    ├─ scan/FolderScanner + scan/TagReader (hilo propio)
  │    └─ analysis/AnalysisQueue (ThreadPool)
  │         └─ analyzeTrack(file) → { bpm, firstBeat, camelot, waveform }
  │              ├─ analysis/BeatAnalyzer (qm-dsp)
  │              ├─ analysis/KeyAnalyzer  (qm-dsp, toCamelot)
  │              └─ analysis/WaveformBuilder
  │    callAsync ◄──── resultado ──── TrackRepository::saveAnalysis
  ├─ AppView / LibraryComponent ◄── setTracks(filtradas y ordenadas)
  └─ AudioEngine (sin cambios)
```

## Tareas

### T1 – Dependencias y ADR
- `cmake/Dependencies.cmake`: SQLite, TagLib + utfcpp, qm-dsp (versiones
  fijas). `cmake/QmDsp.cmake`, `cmake/Sqlite.cmake`.
- ADR 0002, SPEC.md y CLAUDE.md (stack), README (stack, licencias),
  `.gitignore` si aparece algo nuevo.
- Compila en macOS y Windows (CI) antes de seguir: riesgo principal.

### T2 – `turbito_library`: base de datos
- `library/db/Database` (abrir, WAL, `exec`, sentencias preparadas con
  RAII), `Schema` (migraciones), `TrackRepository` (insertar carpeta y
  pistas, listar, guardar análisis, leer waveform, historial).
- `tests/library/db/`: BD en archivo temporal; migración desde 0;
  insertar/leer; ruta duplicada ignorada; guardar análisis; reabrir y
  conservar datos.

### T3 – Escaneo y tags
- `library/scan/TagReader` (TagLib + duración por JUCE) y
  `FolderScanner` (recursivo, extensiones soportadas).
- Pruebas con `test-audio/` (5 WAV, duraciones conocidas: 30 s, 1.6 s…)
  y un WAV con tags escrito en la prueba (INFO via JUCE).

### T4 – Análisis
- `BeatAnalyzer`, `KeyAnalyzer` + `toCamelot`, `WaveformBuilder`,
  `analyzeTrack` (una pasada por bloques).
- Pruebas con `test-audio/README.md` (valores en `tests/TestTracks.h`):
  - `beat_124bpm_30s.wav` → 124.0 ± 0.1, primer beat 0.000 ± 0.02 s.
  - `beat_126bpm_30s.wav` → 126.0 ± 0.1.
  - `tono_la_menor_8A_120bpm.wav` → 8A y 120.0 ± 0.1.
  - `toCamelot` para las 24 tonalidades; waveform: tamaño = duración·20,
    silencio inicial de `silencios_120bpm.wav` = ceros los primeros 3 s.
- `AnalysisQueue`: prueba de que analiza N archivos y entrega N
  resultados en el message thread.

### T5 – Búsqueda, filtro y orden (puros)
- `library/Query.h`: `matches(track, text)` (título, artista, BPM como
  texto, Camelot; sin distinguir mayúsculas/acentos), filtros del mockup,
  `compatibleKeys`, comparadores por columna.
- `tests/library/QueryTests.cpp`.

### T6 – UI de la biblioteca conectada
- `LibraryComponent`: buscador y filtro activos (`onQueryChanged`),
  orden por cabecera, Importar = carpeta (`onImportFolder`), Analizar
  (`onAnalyze`), colecciones con cuentas reales, drag de filas.
- `DeckComponent`/`WaveformView` como destino de drop; `AppView`
  `DragAndDropContainer`.
- `WaveformView`: helper puro `barsAround(peaks, position, 160)` probado.
- `Strings.h`: textos nuevos (p. ej. "Analizando N…" si hace falta; solo
  textos que ya existan en el mockup o se propongan).
- Pruebas de UI existentes en verde; snapshot del mockup sin cambios.

### T7 – `app`: `Library` + motor
- `MainComponent` crea `Library`, carga pistas al arrancar, conecta
  callbacks, encola análisis tras importar, carga en deck con datos del
  registro + historial, waveform con scroll en el timer de 60 Hz, BPM
  master.
- Aceptación manual: importar una carpeta real, ver BPM/tono llenarse
  sin cortes de audio mientras suena una pista, buscar, ordenar, filtrar,
  arrastrar a B, cerrar y abrir (datos persisten).

### T8 – Documentación y cierre
- README (estado fase 5, estructura con `library/`, dependencias),
  CHANGELOG `[Unreleased]`, checklist del plan, `graphify update .`.
  Push y PR a `develop`; el usuario mergea.

## Checkpoints
- Después de T1: CI verde con las dependencias nuevas en ambos SO.
- Después de T7: el usuario prueba con su música.
- "Build & Test" en verde en macOS y Windows antes de pedir el merge.

## Lo que no entra
- Silencios (datos para "Limpiar silencios"): fase 10, con nueva
  migración.
- Dibujar el beatgrid en la waveform (no está en el mockup; se propone).
- BPM que sigue al pitch, sync: fase 6. Hot cues persistentes: fase 7.
- Playlists y listas de Auto Mix: fase 10. Placas y drops: fase 9.
- Vigilar carpetas por cambios, borrar pistas o carpetas de la
  biblioteca, editar tags, reanálisis con otra versión del algoritmo.

## Verificación
1. `cmake --preset macos-debug && cmake --build --preset macos-debug &&
   ctest --preset macos-debug` en verde (incluye BPM/tono de `test-audio`).
2. App: importar carpeta, análisis en fondo sin dropouts con un deck
   sonando, búsqueda/filtro/orden, cargar por botón, doble clic y drag,
   reiniciar y comprobar persistencia.
3. Captura `app-1440x900.png` sin regresiones frente al mockup.
4. Push y `gh run watch`: macOS y Windows en verde.

## Checklist
- [x] T1 – Dependencias y ADR
- [ ] Checkpoint: CI verde con dependencias nuevas
- [x] T2 – Base de datos
- [x] T3 – Escaneo y tags
- [x] T4 – Análisis
- [x] T5 – Búsqueda, filtro y orden
- [ ] T6 – UI de la biblioteca conectada
- [ ] T7 – `app`: `Library` + motor
- [ ] Checkpoint: el usuario prueba con su música
- [ ] T8 – README, CHANGELOG y PR a `develop`
- [ ] "Build & Test" en verde en macOS y Windows
