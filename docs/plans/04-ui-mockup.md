# Fase 4 – UI fiel al mockup (plan)

## Contexto

La fase 3 dejó dos decks y el mezclador sonando con una UI provisional de
widgets JUCE. La fase 4 del spec pide: **`LookAndFeel`, tokens y todos los
componentes; validar con captura contra el mockup.**

La referencia es `design/mockup-reference.html` a 1440 × 900. Ese HTML no
se puede abrir solo (depende del `support.js` del editor de diseño), así que
la comparación visual usa la exportación del canvas:
`design/screenshots/mockup-1440x900.png` (2880 × 1800, escala 2×).

Esta fase dibuja **toda** la ventana del mockup. Solo se conectan al motor
las funciones que ya existen (fases 2 y 3); el resto se dibuja igual que en
el mockup pero sin acción hasta su fase.

Se conecta en fase 4:
- Play/Pausa y CUE; Gain, EQ, Filtro, faders y crossfader.
- Vúmetros, tiempo, progreso y jog girando con la posición.
- Cargar pista en A/B desde la tabla; Ajustes (dispositivo de audio).
- Estado del audio en la barra superior.

Se dibuja sin acción todavía (fase que lo conecta):
- Pitch, SYNC, KEYLOCK (6). Loops, LOOP IN/OUT/RELOOP (7).
- Unidad FX y pads FX (8). Pads Placas y volumen de Placas (9).
- Lógica de Auto Mix (10). Grabar (13).
- Analizar, buscador y filtro de la biblioteca (5).

Decisiones de diseño:
- **Medidas fijas sacadas del CSS del mockup** (padding de ventana 10,
  separación 10, según `tokens.json`; el 12 del spec es el padding interno
  de los paneles). Alturas fijas, medidas en `mockup-1440x900.png`:
  - Barra superior: 52 (7 + logo 36 + 7 + bordes).
  - Waveforms: 132 (8 + 54 + 6 + 54 + 8 + bordes).
  - Fila decks/mezclador: 400. La fija el mezclador (columna de 5 knobs,
    FX, Placas y crossfader); los decks se estiran a esa altura y les
    sobra aire bajo los pads, como en el mockup.
  - Biblioteca: el resto (266 a 1440 × 900; 86 al mínimo 1280 × 720).
  - Decks: `(ancho − 20 − 300 − 20) / 2` cada uno; mezclador 300.
- **Tokens generados desde `design/tokens.json`**: `cmake/Tokens.cmake` lee
  el JSON con `string(JSON …)` en tiempo de configuración y escribe
  `ui/look/Tokens.h` en el build. Una sola fuente de verdad; se quita el
  `shortcut:` del header copiado a mano.
- **Colores del mockup que faltan en `tokens.json`**: se agregan con el
  valor exacto del mockup (no son colores nuevos):
  - `recordBorder` `#4A1F2A`, `recordText` `#FF7A88` (botón Grabar).
  - `textSoft` `#D0CADD` (columna Artista, pestañas sin elegir).
  - `borderStrong` `#433C5C` (punto de Auto Mix en pausa).
  - `switchOff` `#6E6787` (punto del interruptor en pausa).
  - `playhead` `#FFFFFF`.
  - Transparencias como alfa sobre un token, sin color propio: loop activo
    = acento al 16 %; pista del interruptor = acento al 25 %; sombra de
    menús = negro al 45 %.
- **Fuentes y logo como `BinaryData`** (`cmake/BinaryData.cmake`,
  `juce_add_binary_data`): Inter, JetBrains Mono, Titan One y
  `assets/brand/turbito-horizontal.svg` (ya sin `<text>`, todo en paths;
  se dibuja con `juce::Drawable::createFromSVG` a 168 × 36).
  - Riesgo: los TTF son variables. Si JUCE no elige el peso del eje `wght`
    (Inter 400/500/600/700, Mono 500/700), se reemplazan por las instancias
    estáticas oficiales (misma licencia OFL). Se comprueba en T2 con una
    prueba.
- **Componentes propios sobre widgets de JUCE donde basta**: `Knob` es un
  `juce::Slider` rotario (arrastre vertical, rueda fina y doble clic al
  valor por defecto ya vienen con él) dibujado por
  `TurbitoLookAndFeel::drawRotarySlider`. Faders, crossfader, pitch y
  volumen de Placas son `juce::Slider` lineales imitando el `<input
  type=range>` con `accent-color` del mockup (pista fina, pulgar redondo en
  el acento). Tabla con `juce::TableListBox`, cola con `juce::ListBox`,
  menús de Auto Mix con `juce::PopupMenu` (232 de ancho, ítems de 32,
  cabeceras y separadores), selects con `juce::ComboBox`, buscador con
  `juce::TextEditor`.
- **La UI no conoce el motor**: cada componente recibe datos por setters
  simples y emite `ActionEvent` por un `std::function` que le da `app`.
  Así la captura se arma con los datos del mockup sin motor ni audio.
- **`ui/AppView`** contiene toda la ventana (barra, waveforms, decks,
  mezclador, biblioteca). `MainComponent` (en `app`) es `AppView` + motor:
  un `Timer` a 60 Hz copia el `EngineSnapshot` a los componentes.
- **Jog**: gira a 33⅓ rpm (1.8 s por vuelta) según la posición del deck.
- **Vúmetro de 14 segmentos**: segmento `k` (0 abajo, 13 arriba) se enciende
  si el pico ≥ `−3 · (13 − k)` dBFS; verde 0–8, amarillo 9–11, rojo 12–13
  (los mismos cortes del mockup). Caída y retención en la UI, como en la
  fase 3.
- **Cargar pistas sin biblioteca**: "Importar" abre un selector de archivos
  y agrega las pistas a una lista en memoria que llena la tabla (título =
  nombre del archivo, artista/BPM/tono/duración vacíos o "—"). Los botones
  A/B cargan en el deck. La fase 5 cambia la lista por SQLite. (Confirmado
  por el usuario.)
- **Curva del crossfader**: no está en el mockup. Va en la ventana de
  Ajustes (que tampoco está en el mockup) debajo del selector de audio,
  dibujada con el `LookAndFeel`. (Confirmado por el usuario.)
- **Auto Mix en esta fase es solo UI**: contraer a la franja de 44,
  redimensionar 320–640, interruptor Activo/Pausado, los 4 menús con
  tooltip y marcas de opción, resumen, cola con estado vacío, `+` agrega
  una fila "En cola" y `×` la quita. Nada carga ni mezcla hasta la fase 10.

## Diseño

```
app/MainComponent ── posee ──► AudioEngine, AudioDeviceManager
      │  Timer 60 Hz: EngineSnapshot ──► setters de los componentes
      ▼
ui/AppView
  ├─ topbar/TopBar          logo, controladora, audio, BPM master,
  │                         Grabar, Ajustes
  ├─ widgets/WaveformView ×2
  ├─ deck/DeckComponent ×2  cabecera, progreso, JogWheel, transporte,
  │                         loops, pitch, PadGrid
  ├─ mixer/MixerComponent   2 canales (Knob ×5, VuMeter, fader), FX,
  │                         Placas, crossfader
  └─ library/LibraryComponent
       ├─ colecciones + redimensionador (160–360)
       ├─ buscador, filtro, Analizar, Importar, tabla
       └─ AutoMixPanel + redimensionador (320–640) / franja de 44
  componentes ──ActionEvent──► std::function ──► cola del motor (app)
```

- `src/ui` pasa a ser `turbito_ui` (librería estática, depende de
  `turbito_core` y de JUCE; no enlaza `turbito_engine`).
- `ui/look/TurbitoLookAndFeel` (`LookAndFeel_V4`): botones (radio 8 / 6,
  borde `borderControl`, estados de acento), sliders lineales y rotarios,
  `ComboBox`, `PopupMenu`, `TextEditor`, `ScrollBar`, cabecera de tabla y
  tooltips. Fuentes por defecto Inter.
- `ui/look/Fonts`: `Font ui(float size, int weight)`, `mono(…)`.
- `ui/Strings.h`: todos los textos del mockup.
- Pruebas de la UI enlazan `turbito_ui` sin `turbito_engine`.

## Tareas

### T1 – Tokens generados y assets embebidos
- `tokens.json`: los colores faltantes de arriba.
- `cmake/Tokens.cmake` → `Tokens.h` generado (colores `uint32_t` ARGB,
  arrays para `placas`, `hotCues`, `vu`, `waveformPlayed`; fuentes,
  radios, spacing y layout como `constexpr int`).
- `cmake/BinaryData.cmake` → `turbito_assets` con fuentes y logo.
- `TokensTests` siguen en verde con el header generado.

### T2 – `turbito_ui`, `TurbitoLookAndFeel` y fuentes
- `src/ui/CMakeLists.txt`, `Fonts`, `TurbitoLookAndFeel`.
- `tests/ui/look/FontsTests.cpp`: las tres familias cargan desde
  `BinaryData`; Inter 700 mide más ancho que Inter 400 con el mismo texto
  (prueba de que el peso funciona; si falla, instancias estáticas).

### T3 – Widgets
- `Knob` (26 px canal, 24 px Dry/Wet; ±135°, marca de 2 × 8 en el acento).
- `VuMeter` (14 × 7 × 7, separación 2).
- `JogWheel` (148, aro de 6 `empty`, marca 4 × 18 en el acento, tiempo
  transcurrido 19 y restante 11 en Mono).
- `PadGrid` (pestañas Hot cue/Loop/FX/Placas de 28 y pads 4 × 2 de 44; la
  pestaña cambia la vista; colores por modo según el mockup, vacíos en
  `empty` con "Vacío").
- `WaveformView` (barras con 1 px de separación, jugado/no jugado, playhead
  blanco de 2 px al centro, loop opcional; sin datos: barras de 2 px en
  `empty`, como el deck B del mockup).
- `tests/ui/widgets/`: funciones puras de cada widget — segmentos del
  vúmetro por pico, ángulo del jog por posición, formato de tiempo
  (`2:41`, `-3:31`), ángulo del knob por valor.

### T4 – `DeckComponent`, `MixerComponent` y fila de waveforms
- Deck: letra 36 en el acento, título 15/600, artista 12, BPM 22 Mono,
  "BPM · Tono", progreso de 4, jog, CUE/PLAY 40, SYNC/KEYLOCK 30, loops
  (1/2…16) de 28, LOOP IN/OUT/RELOOP, pitch vertical 26 × 112 con etiqueta,
  `PadGrid`. PLAY sonando = fondo en el acento y texto "PAUSA".
- Mezclador: "Canal A/B" en el acento, 5 knobs, vúmetro y fader 26 × 126;
  FX (select, Dry/Wet, 1/4…2, →A/→B); Placas con porcentaje; crossfader
  con "A" y "B" en el acento.
- Emite las acciones existentes (`DeckX_Play`, `DeckX_Cue`, `ChannelX_*`,
  `Mixer_Crossfader`); CUE envía 1 al pulsar y 0 al soltar.

### T5 – `TopBar` y `LibraryComponent`
- Barra: logo, "Sin controladora" (punto `empty`; la detección MIDI es de
  la fase 12), "Audio: 44.1 kHz · buffer 256" con los valores reales del
  dispositivo, "MASTER" + BPM ("—" sin análisis), Grabar, Ajustes.
- Biblioteca: colecciones fijas del spec (Todas las pistas, Carpetas,
  Historial, Placas y drops) con selección; redimensionadores de 12 con
  línea de 2 (`surfaceHover`, acento al arrastrar; flechas ±16); buscador,
  filtro, Analizar, Importar; tabla con Deck/Título/Artista/BPM/Tono/
  Duración/Cargar, fila del deck cargado en `rowActive` con su insignia.
- Importar → lista en memoria; A/B → callback de carga a `app`.

### T6 – `AutoMixPanel`
- Cabecera "Auto Mix" + "N pistas por sonar", interruptor, botón contraer.
- Barra de 4 botones de icono de 30 (iconos del mockup como `Path`), cada
  uno con tooltip propio encima (fondo `text`, texto `bg`) y su
  `PopupMenu` con las opciones y marcas `•`/`✓` del mockup; resumen a la
  derecha con elipsis.
- Cola: número, título, insignia de estado, "Duplicada", meta, duración,
  `×`; estado vacío con el texto del mockup.
- Franja contraída de 44: flecha, punto (acento o `borderStrong`) y
  "AUTO MIX · N" vertical.

### T7 – `AppView` y `MainComponent` conectado
- `AppView` con el layout fijo; la ventana nunca hace scroll de página.
- `MainComponent` reemplaza la UI provisional: borra `DeckRow`,
  `ChannelStrip` y sus sliders; conserva motor, ajustes guardados, atajos
  (`Espacio`/`Q` A, `Enter`/`P` B) y la ventana de Ajustes, ahora con la
  curva del crossfader debajo del selector de audio.
- Aceptación manual: lo mismo de la fase 3, ahora con la UI final.

### T8 – Captura y validación
- `tests/ui/MockupFixture.h`: los datos de ejemplo del script del mockup
  (pistas, pads, knobs, faders, cola, Auto Mix activo, waveforms con la
  misma fórmula de barras).
- `tests/ui/AppViewSnapshotTests.cpp`: arma `AppView` a 1440 × 900 con el
  fixture, lo pinta en una `juce::Image` a escala 2× (sin ventana; mismo
  tamaño que el PNG del mockup), lo guarda en
  `build/screenshots/app-1440x900.png` y comprueba píxeles en puntos
  conocidos: fondo `bg`, panel de la barra `panel`, letra del deck A en
  `deckA`, play del deck A en `deckA`, punto de Grabar en `record`,
  playhead blanco al centro de cada waveform.
- CI sube la captura de macOS y Windows como artefacto.
- Se copia a `design/screenshots/app-1440x900.png` y se compara lado a lado
  con `mockup-1440x900.png`; las diferencias se corrigen en T3–T6.

### T9 – Documentación y cierre
- `README.md` (estado: fase 4 lista, estructura con `turbito_ui`),
  `CHANGELOG.md` `[Unreleased]`, `.gitignore` si hace falta, esta
  checklist. Push y PR a `develop`; el usuario mergea.

## Checkpoints
- Después de T8: el usuario revisa la captura contra el mockup.
- Después de T7: el usuario prueba la app con audio real.
- "Build & Test" en verde en macOS y Windows antes de pedir el merge.

## Lo que no entra
- Biblioteca real (SQLite, escaneo, análisis, búsqueda, waveforms reales,
  BPM y tono): fase 5.
- Funcionalidad de pitch, sync, keylock, jog con scratch: fase 6.
- Hot cues y loops: fase 7. FX: fase 8. Placas: fase 9. Auto Mix: fase 10.
- Atajos y mapeo completo de teclado y ratón: fase 11. MIDI: fase 12.
- Grabar: fase 13.
- Acentos configurables por el usuario y comparación automática píxel a
  píxel contra el PNG del mockup (el texto de navegador y de JUCE nunca
  coincide exacto; se revisa a ojo).

## Verificación
1. `cmake --preset macos-debug && cmake --build --preset macos-debug &&
   ctest --preset macos-debug` en verde.
2. Captura `app-1440x900.png` lado a lado con `mockup-1440x900.png`:
   coinciden colores, posiciones, tamaños y textos.
3. Abrir la app a 1440 × 900 y a 1280 × 720: sin scroll de página, la
   biblioteca toma el resto; importar, cargar en A y B, sonar, mover
   mezclador y crossfader.
4. Push y `gh run watch`: macOS y Windows en verde.

## Checklist

- [x] T1 – Tokens generados y assets embebidos
- [x] T2 – `turbito_ui`, `TurbitoLookAndFeel` y fuentes
- [x] T3 – Widgets
- [x] T4 – `DeckComponent`, `MixerComponent` y fila de waveforms
- [x] T5 – `TopBar` y `LibraryComponent`
- [ ] T6 – `AutoMixPanel`
- [ ] T7 – `AppView` y `MainComponent` conectado
- [ ] Checkpoint: el usuario prueba la app con audio real
- [ ] T8 – Captura y validación
- [ ] Checkpoint: el usuario revisa la captura contra el mockup
- [ ] T9 – README, CHANGELOG y PR a `develop`
- [ ] "Build & Test" en verde en macOS y Windows
