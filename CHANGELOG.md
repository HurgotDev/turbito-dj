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
