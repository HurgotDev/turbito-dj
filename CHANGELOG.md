# Changelog

Todos los cambios visibles de Turbito DJ se anotan aquí. El formato sigue
[Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/) y las versiones
siguen [SemVer](https://semver.org/lang/es/).

Cada PR agrega sus cambios en `[Unreleased]`, en la categoría que
corresponda: Agregado, Cambiado, Obsoleto, Eliminado, Corregido o Seguridad.
Al publicar una versión, esa sección pasa a ser `## [x.y.z] - AAAA-MM-DD` y
es el texto del Release en GitHub.

## [Unreleased]

### Agregado

- Esqueleto de la app: proyecto CMake con JUCE 9 y ventana vacía de
  1440 × 900 (mínimo 1280 × 720) con el fondo del diseño.
- Prueba que mantiene los valores de la UI sincronizados con
  `design/tokens.json`.
- CI "Build & Test" en macOS y Windows en cada push a `main` y en cada PR,
  con la app descargable durante 7 días.
- Workflow "Release" que, al mergear a `main`, compila ambas plataformas y
  crea un Release en borrador con este changelog.
- Motor de audio mínimo: un deck que carga una pista (WAV, AIFF, FLAC o
  MP3) y la reproduce con Play/Pausa y CUE estándar (mantener CUE en pausa
  previsualiza; CUE sonando vuelve al punto de cue).
- Atajos `Espacio` (Play/Pausa) y `Q` (CUE) para el deck A.
- Ajustes de audio para elegir la salida, la frecuencia y el buffer; la
  elección se recuerda al reabrir la app.
- Controles provisionales (Cargar, Play, CUE, Ajustes de audio) hasta la UI
  del diseño en la fase 4.
- Segundo deck (B) con Play/Pausa y CUE, y atajos `Enter` (Play/Pausa) y
  `P` (CUE).
- Mezclador de 2 canales: gain (±12 dB), EQ de 3 bandas (Agudos, Medios,
  Graves) con kill al mínimo y hasta +6 dB, filtro pasa-bajos/pasa-altos en
  un solo knob, fader de volumen y vúmetro por canal.
- Crossfader con curva ajustable, de suave (potencia constante) a corte.
- Doble clic en cualquier knob o fader lo devuelve a su valor por defecto.
- Interfaz del diseño: barra superior con estado del audio, waveforms,
  decks con jog, pads y controles de loop y pitch, mezclador con knobs y
  vúmetros de 14 segmentos, biblioteca con colecciones y tabla de pistas, y
  panel de Auto Mix contraíble y redimensionable. Pitch, sync, loops,
  efectos, placas, grabar y Auto Mix todavía no actúan.
- "Importar" agrega archivos a la tabla de la biblioteca y los botones A/B
  de cada fila los cargan en el deck.
- La curva del crossfader está en Ajustes y se recuerda al reabrir la app.

### Cambiado

- Los controles provisionales de las fases 2 y 3 se reemplazan por la
  interfaz del diseño; las pistas se cargan desde la biblioteca.

### Corregido

- Los logos SVG (`turbito-horizontal.svg` y `turbito-logo.svg`) ya no se ven
  cortados en Safari/iOS: el texto pasa a trazos vectoriales y no depende de
  la fuente incrustada.
