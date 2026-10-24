<p align="center">
  <img src="assets/brand/turbito-horizontal.svg" alt="Turbito DJ" width="420">
</p>

# Turbito DJ

App DJ nativa para macOS y Windows, inspirada en la cultura picotera de la
costa colombiana. Dos decks, mezclador, efectos, hot cues, loops y un sampler
de placas, que funciona igual con o sin controladora.

> **Estado:** en desarrollo. El repositorio tiene la especificación, el
> diseño y los assets; el código empieza con la fase 1 (esqueleto CMake +
> JUCE).

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

- C++20, JUCE 8 y CMake.
- Audio: CoreAudio (macOS), WASAPI y ASIO (Windows).
- Rubber Band (time-stretch y keylock), aubio o Essentia (BPM y tono).
- SQLite embebido, Catch2 + CTest y GitHub Actions.

Plataformas: macOS 13+ (Universal: arm64 + x86_64) y Windows 10/11 x64.

## Compilar

Pendiente hasta la fase 1. Requisitos previstos: CMake 3.22+, un compilador
C++20 (Xcode 15+ o Visual Studio 2022) y los presets de `CMakePresets.json`.

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
