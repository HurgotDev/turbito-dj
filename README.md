<p align="center">
  <img src="assets/brand/turbito-horizontal.svg" alt="Turbito DJ" width="420">
</p>

# Turbito DJ

App DJ nativa para macOS y Windows, inspirada en la cultura picotera de la
costa colombiana. Dos decks, mezclador, efectos, hot cues, loops y un sampler
de placas, que funciona igual con o sin controladora.

> **Estado:** en desarrollo. Fase 1 (esqueleto) lista: proyecto CMake con
> JUCE, ventana vacía de 1440 × 900 y CI en macOS y Windows. Siguiente: fase 2
> (motor de audio mínimo). Plan de fases en [`docs/SPEC.md`](docs/SPEC.md).

## Funciones del MVP

- Biblioteca de archivos locales (MP3, WAV, FLAC, AIFF) con análisis de BPM,
  beatgrid y tono en notación Camelot.
- 2 decks con play/cue, pitch ±8 %, keylock, sync, hot cues, loops y jog.
- Mezclador de 2 canales: gain, EQ de 3 bandas, filtro, faders, crossfader y
  vúmetros.
- Efectos Echo, Reverb, Filtro y Flanger sincronizados al BPM.
- 8 pads por deck (Hot cue, Loop, FX y Placas) y bus de placas propio.
- Controladora MIDI opcional (Numark Party Mix) y control completo con
  teclado y ratón.
- Grabación del master a WAV y Auto Mix con cola de reproducción.

## Stack

- C++20, JUCE 9 y CMake.
- Audio: CoreAudio (macOS), WASAPI y ASIO (Windows).
- Rubber Band (time-stretch y keylock), aubio o Essentia (BPM y tono).
- SQLite embebido, Catch2 + CTest y GitHub Actions.

Plataformas: macOS 13+ (Universal: arm64 + x86_64) y Windows 10/11 x64.

## Compilar

Requisitos: CMake 3.22+, git y un compilador C++20 (Xcode en macOS, Visual
Studio con "Desarrollo para el escritorio con C++" en Windows). JUCE y Catch2
se descargan solos en el primer configure (`cmake/Dependencies.cmake`).

macOS:

```sh
cmake --preset macos-debug           # o macos-release (Universal)
cmake --build --preset macos-debug
ctest --preset macos-debug
open "build/macos-debug/src/TurbitoDJ_artefacts/Debug/Turbito DJ.app"
```

Windows:

```sh
cmake --preset windows
cmake --build --preset windows-debug  # o windows-release
ctest --preset windows-debug
```

El build Release de macOS es Universal (arm64 + x86_64) con LTO, y su enlace
tarda varios minutos.

## CI

Hay un workflow por plataforma (`CI macOS` y `CI Windows`) y ninguno corre
solo: se lanzan a mano desde la pestaña Actions ("Run workflow") o con
`gh workflow run ci-macos.yml` / `gh workflow run ci-windows.yml`. Antes de
cerrar cada fase deben quedar los dos en verde.

## Estructura del repositorio

```
docs/         SPEC.md, planes por fase y decisiones (ADRs)
design/       mockup-reference.html y tokens.json: la referencia de la UI
assets/       logo, ícono y fuentes (Inter, JetBrains Mono, Titan One)
mappings/     mappings MIDI por controladora
test-audio/   audios sintéticos con valores esperados para las pruebas
src/          app, core, engine, library, automix, input, ui
tests/        pruebas con la misma estructura que src/
```

El detalle de carpetas y las reglas de dependencias entre módulos están en
[`CLAUDE.md`](CLAUDE.md). La especificación completa está en
[`docs/SPEC.md`](docs/SPEC.md).

## Licencia

Copyright © 2026 Hugo Gutierrez. Software libre bajo la
[GNU AGPL v3.0](LICENSE): puedes usarlo, modificarlo y redistribuirlo, siempre
que publiques tus cambios bajo la misma licencia. El motivo de la elección
está en [`docs/decisions/0001-license.md`](docs/decisions/0001-license.md).
Las fuentes usan la licencia OFL (ver `assets/fonts/`).

## Contribuir

Las contribuciones son bienvenidas: abre un issue o un pull request. Antes de
proponer cambios, lee [`CLAUDE.md`](CLAUDE.md) (reglas del audio,
arquitectura y estructura) y [`docs/SPEC.md`](docs/SPEC.md). Al contribuir
aceptas que tu código se publique bajo AGPLv3.
