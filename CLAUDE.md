# Turbito DJ

App DJ nativa para macOS y Windows, inspirada en la cultura picotera de la costa colombiana.
La especificación completa está en `docs/SPEC.md`. Léela antes de planear cualquier fase.

## Stack
- C++20, JUCE 8, CMake (`juce_add_gui_app`). macOS 13+ (Universal) y Windows 10/11 x64.
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
- Estructura: `src/engine/`, `src/ui/`, `src/input/`, `src/library/`, `src/automix/`, `tests/`, `mappings/`.
- Cada fase termina compilando en macOS y Windows, con pruebas en verde y un commit.
- Audios de prueba con valores esperados en `test-audio/README.md`: úsalos en las pruebas del análisis y del motor.

## Pendientes conocidos
- Confirmar el modelo exacto de la Numark Party Mix antes de la fase de mapping MIDI.
- Decidir licencia (GPL o comercial): afecta a JUCE y Rubber Band.
