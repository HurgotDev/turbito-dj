# Turbito DJ – Especificación v1

Oct 24, 2026 · @Hugo Gutierrez

## Visión y alcance

Turbito DJ es una app DJ nativa para macOS y Windows, con dos decks, mezclador, efectos, hot cues, loops y un sampler de placas, que funciona igual con o sin controladora. La interfaz debe replicar exactamente el mockup aprobado en el canvas "Turbito DJ – Preview UI".

**Dentro del MVP**

- Biblioteca de archivos locales (MP3, WAV, FLAC, AIFF) con análisis de BPM, beatgrid y tono.
- 2 decks con play/cue, pitch ±8%, keylock, sync, hot cues, loops y jog.
- Mezclador de 2 canales con gain, EQ de 3 bandas, filtro, faders, crossfader y vúmetros.
- Unidad de efectos (Echo, Reverb, Filtro, Flanger) sincronizada al BPM.
- 8 pads por deck con 4 modos: Hot cue, Loop, FX y Placas.
- Bus de placas independiente con su propio volumen.
- Controladora MIDI opcional, con mapping inicial para la Numark Party Mix.
- Control completo con teclado y ratón.
- Grabación del master a WAV y Auto Mix con cola de reproducción.

**Fuera del MVP**

- Integración con servicios de streaming (Spotify, Apple Music, Deezer y otros).
- Visuales o VJ, stems, timecode y más de 2 decks.

## Stack técnico

C++20 sobre JUCE 9, compilado con CMake. Un solo código base genera la app para macOS (Universal: arm64 + x86\_64) y Windows x64.

| Capa | Elección | Motivo |
| --- | --- | --- |
| Framework | JUCE 9 (CMake, `juce_add_gui_app`) | Audio de baja latencia, MIDI, UI y empaquetado multiplataforma |
| Audio | CoreAudio (macOS), WASAPI y ASIO (Windows) vía `AudioDeviceManager` | Latencia objetivo menor a 10 ms |
| Decodificación | `AudioFormatManager` de JUCE (MP3, WAV, FLAC, AIFF) | Incluido en JUCE |
| Time-stretch y keylock | Rubber Band Library (R3) | Calidad profesional; se usa bajo GPL (compatible con AGPLv3) |
| BPM y beatgrid | aubio o Essentia | Detección de tempo y onsets |
| Tono | Essentia (`KeyExtractor`), notación Camelot | Mostrar tonos como 8A o 5A |
| Base de datos | SQLite (`sqlite3` embebido) | Biblioteca, análisis, hot cues y playlists |
| Pruebas | Catch2 + CTest | Pruebas unitarias del motor |
| CI | GitHub Actions: "Build & Test" (macOS y Windows) y "Release" | Build y pruebas en cada push a main y en cada PR; releases en borrador a demanda |

Las fuentes de la UI son Inter y JetBrains Mono, y Titan One solo para el logotipo, empaquetadas como `BinaryData`.

## Arquitectura

&#91;embedded content: arquitectura · entrada, motor de audio y biblioteca\]

MIDI, teclado y ratón alimentan la misma capa de acciones, por eso la controladora es opcional. El motor corre en el hilo de audio; la UI y la biblioteca se comunican con él solo mediante colas lock-free.

## Requisitos funcionales

Cada módulo se considera terminado cuando cumple todos sus puntos, tanto con controladora como solo con teclado y ratón.

**Biblioteca**

- Importar carpetas y escanear subcarpetas, leyendo título, artista y duración de los tags.
- Analizar en segundo plano BPM, beatgrid, tono y forma de onda; los resultados se guardan en SQLite.
- Colecciones: Todas las pistas, Carpetas, Playlists, Historial y Placas y drops.
- Búsqueda instantánea por título, artista, BPM o tono, y orden por cualquier columna.
- Cargar una pista en el deck A o B con botón, doble clic o arrastrando.

**Decks (A y B)**

- Play/pausa y CUE con el comportamiento estándar (mantener CUE = previsualizar).
- Pitch de ±8% (ampliable a ±16%) con keylock activable; el BPM mostrado se recalcula en vivo.
- SYNC: iguala el BPM y alinea la fase con el otro deck.
- Jog: scratch con la pista en pausa o en modo vinilo, y pitch bend con la pista sonando.
- Waveform con scroll, playhead centrado, beatgrid, marcadores de hot cue y loop activo.

**Hot cues y loops**

- 8 hot cues por pista, cuantizados al beat, persistentes en SQLite y con color.
- Auto-loops de 1/8 a 32 beats, loop manual con IN/OUT y RELOOP.

**Mezclador y efectos**

- Por canal: gain, EQ de 3 bandas con kill, filtro LPF/HPF en un solo knob, fader y vúmetro.
- Crossfader con curva configurable.
- Unidad de efectos con Echo, Reverb, Filtro y Flanger, tiempos de 1/4 a 2 beats, Dry/Wet y asignación a A o B.

**Placas (sampler)**

- 8 slots por banco que reproducen audios cortos (drops, jingles, saludos) sobre el master.
- Modos one-shot y gate por slot; asignación de audios desde la colección Placas y drops.
- Bus propio con volumen independiente, sin pasar por el EQ de los canales.

**Grabación**

- Grabar el master a WAV 24 bits con el botón Grabar.

**Auto Mix**

Una cola de pistas que se cargan y reproducen solas, una tras otra, con transición automática.

- **Modo de decks** (uno de dos):
  - *Alternar decks*: la siguiente pista se carga en el deck libre y cada transición pasa de A a B o de B a A.
  - *Un solo deck*: el deck activo prepara la siguiente pista en segundo plano y la lanza cuando le toca, dejando el otro deck libre para mezclar a mano.
- **Tipo de transición** (combinables):
  - *Normal*: cada pista suena de inicio a fin.
  - *Limpiar silencios*: se saltan los silencios al inicio y al final, usando los datos del análisis.
  - *Auto emparejar BPM*: la siguiente pista ajusta su tempo y fase a la actual antes de entrar.
- **Orden de reproducción**:
  - *Aleatorio*: elige la siguiente pista al azar sin cambiar el orden de la lista.
  - *Revolver*: reordena la lista de forma permanente; quedan las mismas pistas en un orden nuevo.
- **Limpieza**: limpiar la lista completa, quitar las ya tocadas y quitar duplicadas.
- **Auto quitar tocadas**: opción para que cada pista salga de la lista al terminar.
- **Guardar y cargar**: las listas de Auto Mix se guardan con nombre en SQLite y se cargan desde la biblioteca.

* **Durante la transición**: cualquier acción del usuario se ignora hasta que termina.
* **Duración de la transición**: configurable. Con Emparejar BPM se mide en beats: 8, 16 (por defecto) o 32. Sin emparejar se mide en segundos: 4, 8 (por defecto) o 12.
* **UI**: panel lateral derecho de la biblioteca, contraíble a una franja de 44 px y redimensionable (320–640 px), con un interruptor discreto de Activo/Pausado, el estado, la cola y las opciones en 4 menús desplegables por categoría (Modo de decks, Transición, Orden de reproducción y Lista). Cada menú se abre con un botón de solo icono que muestra su nombre en un tooltip al pasar el cursor o al enfocarlo. Cada pista de la biblioteca tiene un botón + para agregarla a la cola.

## Interfaz: fiel al mockup

La UI se implementa pixel a pixel desde el mockup aprobado. El archivo fuente del mockup va en el repo como `design/mockup-reference.html` y es la referencia de verdad para medidas, colores y textos. Ningún cambio visual se hace sin actualizar primero el mockup.

**Tokens de color**

| Token | Valor | Uso |
| --- | --- | --- |
| `bg` | `#0A0A12` | Fondo de la ventana, waveforms y jogs (negro noche del logo) |
| `panel` | `#14121F` | Paneles (barra superior, decks, mezclador, biblioteca) |
| `surface` | `#1D1A2B` | Botones en reposo, encabezado de tabla, menús |
| `border` | `#2A2639` | Bordes de paneles |
| `borderControl` | `#34304A` | Bordes de botones y knobs |
| `empty` | `#262236` | Pads vacíos, segmentos apagados, aro del jog |
| `text` | `#F2EFF7` | Texto principal |
| `textMuted` | `#ABA4C0` | Etiquetas y texto secundario |
| `deckA` | `#00D9FF` (cian) | Acento del deck A (configurable) |
| `deckB` | `#FF2E9A` (magenta) | Acento del deck B (configurable) |
| `placas` | `#FFE500` / `#FFC93C` | Pads y bus de placas |
| `fx` | `#FF7A00` | Pads en modo FX |
| `loop` | `#39FF6A` | Pads en modo Loop |
| `hotCues` | cian, lima, amarillo, magenta, naranja, rojo | Colores de hot cues 1–7 |
| `vuLow` / `vuMid` / `vuHigh` | `#39FF6A` / `#FFE500` / `#FF2D3F` | Vúmetro y botón Grabar |

La paleta de la UI sale del logotipo de Turbito DJ: neutros con tinte violeta sobre el negro noche, y los acentos fluorescentes de la pintura picotera. El logo horizontal va en la esquina izquierda de la barra superior.

**Tipografía y forma**

- Inter 400/500/600/700 para texto; JetBrains Mono 500/700 para BPM, tiempos y valores.
- Tamaños: títulos de pista 15, BPM 22, tiempos del jog 19, botones 12–13, etiquetas 11 en mayúsculas con tracking 0.08em.
- Radios: paneles 12, botones y pads 8, botones pequeños 6. Separación entre paneles 12.

**Layout (ventana de referencia 1440 × 900)**

1. Barra superior: logo y nombre, estado de controladora, audio, BPM master, Grabar y Ajustes.
2. Waveforms de A y B apiladas, 54 px de alto cada una, playhead blanco al centro.
3. Fila central: Deck A, Mezclador (300 px) y Deck B. Los decks ocupan el resto a partes iguales.
4. Biblioteca: tres paneles que ocupan todo el alto restante, al estilo Virtual DJ. A la izquierda las colecciones (redimensionable, 160–360 px); al centro el buscador, el filtro y la tabla de pistas; a la derecha el panel de Auto Mix, contraíble y redimensionable.

La interfaz siempre ocupa la ventana completa y nunca hace scroll de página: la barra superior, las waveforms y la fila de decks y mezclador tienen altura fija, y la biblioteca toma el alto restante. Cada panel (colecciones, pistas y cola de Auto Mix) tiene su propio scroll interno. El tamaño mínimo de ventana es 1280 × 720. Todo se dibuja con un `LookAndFeel` propio y componentes personalizados (`DeckComponent`, `JogWheel`, `Knob`, `PadGrid`, `WaveformView`, `VuMeter`).

## Controladora opcional

La app funciona completa sin controladora: toda acción pasa por una capa de entrada común, y el MIDI, el teclado y el ratón son solo fuentes distintas de las mismas acciones.

- **Acciones**: un enum `Action` (por ejemplo `DeckA_Play`, `DeckB_HotCue3`, `Mixer_Crossfader`) con un valor normalizado de 0 a 1 o relativo para jogs.
- **Conexión en caliente**: se detecta al conectar o desconectar el dispositivo. La barra superior muestra "Numark Party Mix conectada" o "Sin controladora" sin interrumpir el audio.
- **Mappings**: un archivo JSON por controladora en `mappings/`, con mensajes MIDI de entrada (botones, faders, jogs) y de salida (LEDs). El primero es `numark-party-mix.json`.
- **Modelo exacto**: pendiente confirmar la versión de la Party Mix (Party Mix, MKII, Live o II) en la etiqueta inferior. El mapping se obtiene con un monitor MIDI y se contrasta con los mappings de Mixxx.
- **Modo MIDI Learn** (fase posterior): el usuario toca un control físico y luego hace clic en el elemento de la UI que quiere asignarle.

**Atajos de teclado por defecto**

| Acción | Deck A | Deck B |
| --- | --- | --- |
| Play/pausa | `Espacio` | `Enter` |
| CUE | `Q` | `P` |
| SYNC | `W` | `O` |
| Hot cues 1–4 | `1` `2` `3` `4` | `7` `8` `9` `0` |
| Pitch bend − / + | `A` / `S` | `K` / `L` |
| Placas 1–8 | `F1`–`F8` | `F1`–`F8` |
| Crossfader izq. / der. | `←` / `→` | `←` / `→` |

Con el ratón, los knobs y faders se arrastran verticalmente, la rueda hace ajuste fino y doble clic los regresa a su valor por defecto.

## Requisitos no funcionales

El audio nunca debe cortarse: el hilo de audio no reserva memoria, no bloquea y no toca la UI ni la base de datos.

| Área | Criterio |
| --- | --- |
| Latencia | Menor a 10 ms de ida y vuelta con buffer de 256 a 44.1 kHz |
| Estabilidad | 2 horas de mezcla continua sin dropouts en un equipo de gama media |
| UI | 60 fps en waveforms y jogs; la UI lee el estado del motor con atomics o colas lock-free |
| Análisis | En hilos de fondo; nunca bloquea la reproducción |
| Arranque | La app abre en menos de 3 segundos con 5,000 pistas en la biblioteca |
| Plataformas | macOS 13+ (Apple Silicon e Intel) y Windows 10/11 x64 |
| Calidad de código | Pruebas unitarias del motor (pitch, loops, sync, cuantización) en CI |

**Criterio de aceptación de la UI**: una captura de la app a 1440 × 900 se compara lado a lado con el mockup y coinciden colores, posiciones, tamaños y textos.

## Plan de fases para Claude Code

Cada fase termina con algo ejecutable y probado. Se trabaja una fase por sesión de Claude Code, empezando en modo plan y cerrando con commit.

1. **Esqueleto**: proyecto CMake con JUCE, ventana vacía de 1440 × 900, CI en GitHub Actions para macOS y Windows, `CLAUDE.md` con las reglas del proyecto.
2. **Motor de audio mínimo**: un deck que carga un archivo y hace play, pausa y CUE; selector de dispositivo de audio.
3. **Dos decks + mezclador**: gain, EQ, filtro, faders, crossfader y vúmetros, con pruebas unitarias.
4. **UI fiel al mockup**: `LookAndFeel`, tokens y todos los componentes; validar con captura contra el mockup.
5. **Análisis y biblioteca**: SQLite, importación de carpetas, BPM, beatgrid, tono y forma de onda.
6. **Pitch, keylock y sync**: Rubber Band, pitch fader, SYNC con alineación de fase y jog.
7. **Hot cues y loops**: cuantizados, persistentes y visibles en la waveform.
8. **Efectos**: Echo, Reverb, Filtro y Flanger sincronizados al BPM.
9. **Placas**: sampler de 8 slots, bus propio y gestión desde la colección Placas y drops.
10. **Auto Mix**: cola, modos de decks, transiciones, aleatorio, revolver, limpieza y listas guardadas.
11. **Entrada**: capa de acciones, atajos de teclado y control con ratón; la app queda usable sin controladora.
12. **Party Mix**: mapping MIDI de entrada y LEDs, detección en caliente.
13. **Grabación y empaquetado**: grabar el master a WAV, firma y notarización en macOS, instalador en Windows.

**Archivos para el repo al empezar**

- [ ] `docs/SPEC.md`: este documento exportado a Markdown.
- [ ] `design/mockup-reference.html`: el fuente del mockup.
- [ ] `CLAUDE.md`: stack, reglas del hilo de audio, tokens y la regla "la UI debe coincidir con `design/mockup-reference.html`".

