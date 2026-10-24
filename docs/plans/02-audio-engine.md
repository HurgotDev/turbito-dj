# Fase 2 – Motor de audio mínimo (plan)

## Contexto

La fase 1 dejó una ventana vacía con CI en verde. La fase 2 del spec pide:
**un deck que carga un archivo y hace play, pausa y CUE; selector de
dispositivo de audio.** Es la base del motor: aquí nacen `src/core` y
`src/engine` como librerías estáticas, con las reglas del hilo de audio
(sin reservar memoria, sin locks) desde el primer día.

Decisiones tomadas con el usuario:
- UI temporal con widgets JUCE básicos (Cargar, Play/Pausa, CUE, tiempo,
  Ajustes de audio) sobre fondo `bg`. Se tira en la fase 4.
- ASIO queda para después: fase 2 con CoreAudio y WASAPI.

Decisiones de diseño:
- **Pista precargada en memoria**: el archivo se decodifica entero a un
  `juce::AudioBuffer<float>` en un hilo de fondo. Simplifica scratch y loops
  más adelante y evita I/O en el hilo de audio. (~100 MB por pista de 5 min.)
- **Lectura con posición fraccional** y ratio `fileRate / deviceRate` con
  interpolación lineal: resuelve archivos a 44.1 kHz en un dispositivo a
  48 kHz y deja lista la base del pitch. Comentario `shortcut:` (calidad
  lineal, se reemplaza con Rubber Band en la fase 6).
- **Un solo productor por cola**: todo lo que va al motor sale del hilo de
  mensajes; el hilo de carga entrega la pista con `MessageManager::callAsync`.
- **Selección de dispositivo persistente**: `AudioDeviceManager::
  createStateXml` guardado en `juce::PropertiesFile` al cerrar.

## Diseño

```
UI (mensajes) ──ActionEvent──►  SpscQueue  ──► AudioEngine (hilo de audio)
UI ──Track*──► SpscQueue(load) ──►  Deck  ──Track* viejo──► SpscQueue(retired)
UI ◄── DeckSnapshot (atomics: loaded, playing, position, length) ◄── Deck
```

- `src/core` → `turbito_core`:
  - `Action.h`: `enum class Action { DeckA_Play, DeckA_Cue }` y
    `struct ActionEvent { Action action; float value; }` (CUE: 1 = pulsado,
    0 = soltado; Play: 1 = pulsado).
  - `SpscQueue.h`: plantilla `SpscQueue<T, Capacity>` sobre
    `juce::AbstractFifo` con almacenamiento fijo; `push`/`pop` devuelven
    `bool` (llena / vacía).
  - `Track.h`: `struct Track { juce::AudioBuffer<float> audio; double
    sampleRate; juce::String name; }`.
  - `DeckSnapshot.h`: atomics escritos por el motor y leídos por la UI.
- `src/engine` → `turbito_engine` (→ `core`):
  - `deck/Deck.h/.cpp`: estado de transporte y `render(float* const*,
    numChannels, numSamples, deviceRate)`. Sin asignaciones.
  - `deck/TrackLoader.h/.cpp`: `std::unique_ptr<Track> loadTrack(const
    juce::File&)` con `AudioFormatManager` (formatos básicos: WAV, AIFF,
    FLAC, MP3). Síncrono; quien llama lo corre fuera del hilo de audio.
  - `AudioEngine.h/.cpp`: `juce::AudioIODeviceCallback`; por bloque vacía
    las colas de acciones y cargas, renderiza el deck y publica el snapshot.
    Expone `post(ActionEvent)`, `load(std::unique_ptr<Track>)` y
    `collectRetired()` (el hilo de mensajes borra las pistas viejas).
- **JUCE compartido**: hoy la app y las pruebas enlazan módulos JUCE por
  separado. Con varias librerías estáticas eso duplica símbolos, así que se
  crea un target `turbito_juce` (librería estática con los módulos JUCE
  enlazados `PRIVATE` y sus include dirs y definiciones propagados como
  `INTERFACE`, patrón "shared code"). App, `turbito_core`, `turbito_engine`
  y `turbito_tests` enlazan solo `turbito_juce`. Las definiciones
  `JUCE_WEB_BROWSER=0`, `JUCE_USE_CURL=0` se mueven ahí.

### Comportamiento de CUE (estándar)

- En pausa, pulsar CUE: fija el punto de cue en la posición actual y
  previsualiza mientras se mantiene; al soltar vuelve al cue y pausa.
- Sonando, pulsar CUE: salta al cue y pausa.
- Manteniendo CUE, pulsar Play: sigue sonando al soltar CUE.
- Play en pausa reproduce desde la posición actual; Play sonando pausa.
- Al llegar al final de la pista se pausa en el final.
- Cargar una pista: posición 0, cue en 0, en pausa.
- Arrancar, parar y saltar al cue aplican un fundido lineal de ~5 ms para
  evitar clics (agregado tras escuchar el render de prueba).

## Tareas

### T1 – Build: `turbito_juce`, `turbito_core`, `turbito_engine`
- `cmake/JuceShared.cmake` (o `src/CMakeLists.txt`): target `turbito_juce`
  con `juce_core`, `juce_audio_basics`, `juce_audio_formats`,
  `juce_audio_devices`, `juce_audio_utils`, `juce_gui_basics`.
- `src/core/CMakeLists.txt`, `src/engine/CMakeLists.txt`.
- App y pruebas pasan a enlazar a través de las librerías.
- Aceptación: `macos-debug` compila sin warnings nuevos y la prueba de
  tokens sigue en verde.

### T2 – Core: `SpscQueue`, `Action`, `Track`, `DeckSnapshot`
- Pruebas `tests/core/SpscQueueTests.cpp`: push/pop en orden, `push` en
  cola llena devuelve `false`, `pop` en vacía devuelve `false`, vuelta
  completa del índice circular.

### T3 – Engine: `TrackLoader`, `Deck`, `AudioEngine`
- `tests/engine/deck/TrackLoaderTests.cpp`: `beat_124bpm_30s.wav` carga con
  2 canales, 44100 Hz y 30 s (±1 bloque); archivo inexistente → `nullptr`.
- `tests/engine/deck/DeckTests.cpp` (con un `Track` sintético y con
  `test-audio`): sin pista o en pausa → silencio; play avanza `numSamples`;
  pausa detiene; los casos de CUE de arriba; final de pista; a 48 kHz la
  posición avanza `44100/48000` por muestra de salida.
- `tests/engine/AudioEngineTests.cpp`: llamar al callback sin dispositivo
  (`audioDeviceAboutToStart` con un `AudioIODevice` falso o llamando a
  `audioDeviceIOCallbackWithContext` directo); cargar dos pistas → la
  primera sale por `collectRetired()`; acciones en cola llegan al deck.

### T4 – App: UI temporal y selector de dispositivo
- `MainComponent`: posee `AudioDeviceManager` (`initialise(0, 2, state,
  true)`) y `AudioEngine`; fila de `TextButton` "Cargar", "Play", "CUE"
  (CUE responde a pulsar y soltar), etiqueta con `mm:ss.d / mm:ss` y
  nombre de pista, botón "Ajustes de audio" que abre un `DialogWindow` con
  `AudioDeviceSelectorComponent`. Un `Timer` a 30 Hz lee el snapshot y
  llama `collectRetired()`.
- Textos en español en `src/ui/Strings.h` (convención "centralizados").
- Teclas: Espacio = Play, Q = CUE (atajos por defecto del spec).
- Aceptación manual: cargar `beat_124bpm_30s.wav`, play/pausa/CUE suenan
  bien, cambiar de salida en Ajustes sin cerrar la app, la selección se
  recuerda al reabrir.

### T5 – Documentación y cierre
- `docs/plans/02-audio-engine.md`: este plan con la checklist.
- `README.md` (estado: fase 2 lista), `CHANGELOG.md` `[Unreleased]`
  (cargar pista, play/pausa/CUE, selector de audio), `.gitignore` si hace
  falta.
- Rama `phase/02-audio-engine` desde `develop`, un commit por tarea, push y
  PR a `develop`; el usuario mergea.

## Checkpoints
- Después de T4: el usuario prueba la app con audio real (captura y oído).
- "Build & Test" en verde en macOS y Windows antes de pedir el merge.

## Lo que no entra
- Segundo deck, mezclador y vúmetros: fase 3.
- UI fiel al mockup: fase 4.
- Pitch, keylock, jog: fase 6. Cue cuantizado: fase 7.
- ASIO, capa de acciones completa con dispatcher y mappings: fases 11–12.
- Streaming desde disco para pistas largas (se precarga entera).

## Verificación
1. `cmake --preset macos-debug && cmake --build --preset macos-debug &&
   ctest --preset macos-debug` en verde.
2. Abrir la app, cargar `test-audio/beat_124bpm_30s.wav`, probar
   play/pausa y los tres casos de CUE; abrir Ajustes de audio y cambiar de
   salida y de buffer (256) sin cortes.
3. Push y `gh run watch`: macOS y Windows en verde.

## Checklist

- [x] T1 – Build: `turbito_juce`, `turbito_core`, `turbito_engine`
- [x] T2 – Core: `SpscQueue`, `Action`, `Track`, `DeckSnapshot`
- [x] T3 – Engine: `TrackLoader`, `Deck`, `AudioEngine`
- [x] T4 – App: UI temporal y selector de dispositivo
- [ ] Checkpoint: el usuario prueba la app con audio real (pendiente:
  prueba manual en macOS)
- [x] T5 – README, CHANGELOG y PR a `develop`
- [x] "Build & Test" en verde en macOS y Windows
