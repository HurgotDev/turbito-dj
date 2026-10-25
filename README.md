<p align="center">
  <img src="assets/brand/turbito-horizontal.svg" alt="Turbito DJ" width="420">
</p>

# Turbito DJ

App DJ nativa para macOS y Windows, inspirada en la cultura picotera de la
costa colombiana. Dos decks, mezclador, efectos, hot cues, loops y un sampler
de placas, que funciona igual con o sin controladora.

> **Estado:** en desarrollo. Fase 4 (UI fiel al mockup) lista: la ventana
> completa del diseño (barra superior, waveforms, decks, mezclador,
> biblioteca y panel de Auto Mix) con Play, Pausa, CUE, mezclador y
> crossfader funcionando; pitch, sync, loops, efectos, placas y Auto Mix se
> ven pero llegan en sus fases. Siguiente: fase 5 (análisis y biblioteca).
> Plan de fases en [`docs/SPEC.md`](docs/SPEC.md).

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
- Rubber Band (time-stretch y keylock), qm-dsp (BPM, beatgrid y tono) y
  TagLib (tags de los archivos).
- SQLite embebido, Catch2 + CTest y GitHub Actions.

Plataformas: macOS 13+ (Universal: arm64 + x86_64) y Windows 10/11 x64.

## Compilar

Requisitos: CMake 3.22+, git y un compilador C++20 (Xcode en macOS, Visual
Studio con "Desarrollo para el escritorio con C++" en Windows). Las
dependencias se descargan solas en el primer configure
(`cmake/Dependencies.cmake`).

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

El desarrollo ocurre en `develop` (rama por defecto); `main` solo recibe
versiones.

- **Build & Test** (`build-and-test.yml`): compila y prueba en macOS (Debug,
  arquitectura nativa, para que sea rápido) y Windows (Release) en cada PR
  hacia `develop` o `main` y en cada push a `develop`. La app compilada queda
  descargable 7 días en la corrida, como build de desarrollo.
- **Release** (`release.yml`): corre solo al mergear un PR a `main` (desde
  `develop` o un `hotfix/*`). Compila las dos plataformas (macOS Universal
  con LTO, unos 35 minutos) en la versión de `CMakeLists.txt` y crea un
  Release en borrador con la sección de esa versión de
  [`CHANGELOG.md`](CHANGELOG.md), para revisarlo y publicarlo.

Las versiones aún no están firmadas: macOS y Windows muestran un aviso la
primera vez que se abre la app. La firma llega con la fase 13.

## Estructura del repositorio

```
docs/         SPEC.md, planes por fase y decisiones (ADRs)
design/       mockup-reference.html, tokens.json y capturas: la referencia
              de la UI
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

Dependencias y sus licencias:

- JUCE: AGPLv3 o comercial (se usa AGPLv3).
- Rubber Band: GPL. qm-dsp: GPLv2 o posterior. ASIO SDK: GPLv3.
- TagLib: LGPL-2.1 o MPL-1.1. utfcpp: BSL-1.0. Catch2 (solo pruebas):
  BSL-1.0.
- SQLite: dominio público.

## Contribuir

Las contribuciones son bienvenidas: abre un issue o un pull request. Antes de
proponer cambios, lee [`CLAUDE.md`](CLAUDE.md) (reglas del audio,
arquitectura, estructura y método de trabajo) y [`docs/SPEC.md`](docs/SPEC.md).

- Trabaja en una rama desde `develop` (`feat/…`, `fix/…`) y abre un PR a
  `develop`.
- "Build & Test" debe quedar en verde en macOS y Windows.
- Anota tu cambio en `[Unreleased]` de [`CHANGELOG.md`](CHANGELOG.md).

Al contribuir aceptas que tu código se publique bajo AGPLv3.
