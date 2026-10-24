# Turbito DJ

App DJ nativa para macOS y Windows, inspirada en la cultura picotera de la costa colombiana.
La especificación completa está en `docs/SPEC.md`. Léela antes de planear cualquier fase.

## Stack
- C++20, JUCE 9, CMake (`juce_add_gui_app`). macOS 13+ (Universal) y Windows 10/11 x64.
- Audio: `AudioDeviceManager` (CoreAudio, WASAPI, ASIO). Decodificación con `AudioFormatManager`.
- Time-stretch/keylock: Rubber Band (R3). BPM/beatgrid: aubio o Essentia. Tono: Essentia (notación Camelot).
- Base de datos: SQLite embebido. Pruebas: Catch2 + CTest. CI: GitHub Actions (macOS y Windows).

## Reglas de oro del audio
- El hilo de audio NUNCA reserva memoria, bloquea (mutex, I/O, logs) ni toca la UI o SQLite.
- La UI lee el estado del motor con atomics o colas lock-free (`juce::AbstractFifo`), nunca al revés.
- El análisis corre en hilos de fondo y nunca bloquea la reproducción.
- Objetivo: latencia < 10 ms con buffer de 256 a 44.1 kHz; 2 h de mezcla sin dropouts.

## Arquitectura
- Entrada: MIDI (controladora opcional), teclado y ratón → **capa de acciones** (`enum class Action` + valor normalizado 0–1 o relativo para jogs). Ningún componente de UI ni mapping MIDI llama al motor directamente.
- Motor (hilo de audio): Deck A y Deck B (pitch, keylock, cues, loops) → Canal A/B (gain, EQ 3 bandas, filtro, FX) → Master (suma + crossfader) → Salida de audio y Grabador WAV. El sampler de **Placas** tiene su propio bus hacia el Master, sin pasar por el EQ.
- Biblioteca: SQLite (pistas, análisis, hot cues, playlists, listas de Auto Mix) + workers de análisis.
- Auto Mix: orquesta carga y transiciones emitiendo acciones; durante una transición se ignoran las acciones del usuario.

## Fidelidad del diseño (obligatorio)
- La UI debe coincidir pixel a pixel con `design/mockup-reference.html` a 1440 × 900.
  Ese archivo es HTML del editor de diseño: lee sus estilos inline como referencia de medidas, colores y textos (no se ejecuta solo).
- Colores, fuentes, radios y medidas: usa siempre `design/tokens.json`. No inventes colores nuevos.
- La ventana nunca hace scroll de página: barra superior, waveforms y fila decks/mezclador con alto fijo; la biblioteca toma el resto y cada panel tiene scroll propio.
- Todo se dibuja con un `LookAndFeel` propio (`TurbitoLookAndFeel`) y componentes propios: `DeckComponent`, `JogWheel`, `Knob`, `PadGrid`, `WaveformView`, `VuMeter`, `AutoMixPanel`.
- Fuentes en `assets/fonts/` (Inter, JetBrains Mono para la UI; Titan One solo para el logo), embebidas como `BinaryData`.
- Logo e ícono en `assets/brand/`. El logo horizontal va en la esquina izquierda de la barra superior.
- Un cambio visual que no esté en el mockup se propone primero; no se implementa por cuenta propia.

## Convenciones
- Código e identificadores en inglés; textos de la UI en español (centralizados para traducir después).
- Estructura: ver la sección "Estructura de carpetas".
- Licencia AGPLv3 (`docs/decisions/0001-license.md`): toda dependencia nueva
  debe ser compatible (GPLv3, LGPL, MIT, BSD, Apache-2.0); nada propietario.
- Cada fase termina compilando en macOS y Windows, con pruebas en verde y un commit.
- `CHANGELOG.md` siempre al día: todo cambio visible para el usuario se anota
  en `[Unreleased]` en el mismo PR (formato Keep a Changelog, en español).
- `README.md` siempre al día: todo cambio que afecte estado, funciones,
  stack, cómo compilar, estructura o licencia lo actualiza en el mismo commit.
- `.gitignore` siempre al día: toda herramienta, carpeta de build, caché o
  archivo generado nuevo se ignora en el mismo commit que lo introduce. Antes
  de cada push, `git status --short` no debe mostrar nada que sobre, y
  `git ls-files -ci --exclude-standard` debe salir vacío.
- Audios de prueba con valores esperados en `test-audio/README.md`: úsalos en las pruebas del análisis y del motor.

## Método de trabajo
- Nunca se hace commit directo a `main` (está protegida). Cada fase o cambio
  independiente va en su rama: `phase/NN-nombre`, `feat/…`, `fix/…`,
  `docs/…`, `ci/…`. Dentro de la rama, un commit por tarea.
- Al terminar: push de la rama y PR a `main` con resumen, cambios y cómo se
  verificó. La CI "Build & Test" (jobs "macOS" y "Windows") corre sola en el
  PR y debe quedar en verde.
- El usuario revisa y hace el merge. Claude nunca mergea un PR.
- Después del merge: sincronizar `main` local y borrar la rama.
- Versiones (SemVer, `0.x` hasta el MVP): un PR "Release x.y.z" sube
  `project(... VERSION)` en `CMakeLists.txt` y pasa `[Unreleased]` a
  `## [x.y.z] - fecha` en `CHANGELOG.md`. Tras el merge, el usuario lanza el
  workflow "Release" (manual), que crea un Release en borrador; el usuario lo
  revisa y lo publica. Claude no lanza "Release" por cuenta propia.

## Estructura de carpetas

```
CMakeLists.txt, CMakePresets.json  # raíz; presets macos-universal, windows-x64
cmake/              # módulos CMake: dependencias (FetchContent), BinaryData
.github/workflows/  # build-and-test.yml (push/PR), release.yml (manual)
docs/
  SPEC.md
  plans/            # un plan por fase: 01-skeleton.md … 13-packaging.md
  decisions/        # ADRs: licencia, aubio vs Essentia, modelo Party Mix
design/             # mockup-reference.html, tokens.json
  screenshots/      # capturas 1440×900 de la app para comparar con el mockup
assets/             # brand/, fonts/ (→ BinaryData)
mappings/           # un JSON por controladora: numark-party-mix.json
test-audio/         # WAV sintéticos con valores esperados
src/
  app/              # Main.cpp, MainWindow, MainComponent: lo conecta todo
  core/             # Action, colas lock-free, snapshots de estado del motor
  engine/           # hilo de audio: AudioEngine
    deck/           # transporte, cues, loops, sync, time-stretch
    mixer/          # canal, EQ, filtro, fader, crossfader, master, VU
    fx/             # FxUnit, Echo, Reverb, Filter, Flanger
    sampler/        # Placas: slots, one-shot/gate, bus propio
    recorder/       # grabador WAV del master
  library/
    db/             # SQLite: conexión, esquema, migraciones, repositorios
    scan/           # importar carpetas y leer tags
    analysis/       # workers: BPM, beatgrid, tono, waveform, silencios
  automix/          # cola, modos, transiciones, orden, listas guardadas
  input/            # dispatcher, teclado, ratón, MIDI, mappings, hot-plug
  ui/
    look/           # TurbitoLookAndFeel, tokens, fuentes
    Strings.h       # todos los textos de la UI en español
    widgets/        # Knob, JogWheel, PadGrid, WaveformView, VuMeter
    topbar/  deck/  mixer/  library/   # incluye AutoMixPanel en library/
tests/              # misma forma que src/; TestAudio.h con valores esperados
```

- Cada carpeta de `src/` es una librería estática de CMake
  (`turbito_core`, `turbito_engine`, …); las pruebas enlazan sin la UI.
- Dependencias permitidas (nada más):
  - `core` no depende de ningún módulo del proyecto.
  - `engine` y `library` → `core`.
  - `automix` e `input` → `core`, `library`; solo emiten `Action`.
  - `ui` → `core`, `library`; lee el motor solo vía snapshots de `core`.
  - `app` es el único que conoce a todos.
- `tests/` copia la forma de `src/`: `src/engine/deck/Deck.cpp` →
  `tests/engine/deck/DeckTests.cpp`.
- Cada fase deja su plan en `docs/plans/NN-nombre.md`; cada decisión
  abierta, un ADR en `docs/decisions/`.
- No se crean carpetas vacías: cada carpeta nace en la fase que la usa.
- Dependencias externas con `FetchContent` y versión fija en `cmake/`
  (sin submódulos); el build va en `build/`, fuera de git.

## Pendientes conocidos
- Confirmar el modelo exacto de la Numark Party Mix antes de la fase de mapping MIDI.

## graphify

This project has a knowledge graph at graphify-out/ with god nodes, community structure, and cross-file relationships.

Rules:
- For codebase questions, first run `graphify query "<question>"` when graphify-out/graph.json exists. Use `graphify path "<A>" "<B>"` for relationships and `graphify explain "<concept>"` for focused concepts. These return a scoped subgraph, usually much smaller than GRAPH_REPORT.md or raw grep output.
- If graphify-out/wiki/index.md exists, use it for broad navigation instead of raw source browsing.
- Read graphify-out/GRAPH_REPORT.md only for broad architecture review or when query/path/explain do not surface enough context.
- After modifying code, run `graphify update .` to keep the graph current (AST-only, no API cost).
- A committed graphify-out/graph.json is rewritten whole on every build, so a large diff to it in a pull request is expected. Do not flag it as churn or suggest skipping the graph rebuild or its hooks.
