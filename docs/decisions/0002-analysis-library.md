# 0002 – Biblioteca de análisis: qm-dsp, TagLib y SQLite

Oct 25, 2026 · Aceptada

## Contexto

La fase 5 necesita, en segundo plano y sin tocar el hilo de audio, BPM,
beatgrid y tono (Camelot) de cada pista, leer título, artista y duración
de los tags de los archivos, y guardar todo en una base de datos local.

Las opciones de análisis que se evaluaron:

- **aubio**: GPLv3. Solo detecta BPM y onsets; para el tono habría que
  añadir libKeyFinder y FFTW. Su build no es CMake (usa waf).
- **Essentia**: AGPLv3 o licencia propietaria. Trae muchas dependencias
  (FFTW, Eigen, Taglib, yaml…) y no tiene build oficial para MSVC, así que
  Windows dependería de un port propio.
- **qm-dsp** (Queen Mary, Universidad de Londres): GPLv2 o posterior. C++
  sin dependencias externas (trae kissfft). Es la base de análisis de
  Mixxx, con BPM y tonalidad (`GetKeyMode`) en la misma librería.

Los tags no los lee JUCE: `AudioFormatReader` da duración y formato, pero
no ID3 ni Vorbis comments.

## Decisión

- **qm-dsp** para BPM, beatgrid y tono. Se compila como `turbito_qmdsp`
  (STATIC) con el subconjunto de fuentes que usan `TempoTrackV2`,
  `DetectionFunction` y `GetKeyMode`, y se fija a un commit concreto
  en `cmake/Dependencies.cmake`.
- **TagLib 2.x** para título, artista y etiquetas. Se usa bajo LGPL-2.1
  (también disponible bajo MPL-1.1). Necesita **utfcpp** (header-only,
  BSL-1.0), que se descarga aparte para no depender de una instalación
  del sistema. Target: `TagLib::tag`.
- **SQLite** como amalgamación oficial (dominio público), descargada por
  URL con hash SHA-256 fijo. Target: `turbito_sqlite` (STATIC, C).

## Consecuencias

- Licencia: qm-dsp se usa bajo GPLv2 o posterior, que permite tomarla
  como GPLv3 y, por tanto, enlazarla en una app AGPLv3.
- TagLib (LGPL-2.1 o MPL-1.1) y utfcpp (BSL-1.0) son compatibles con
  AGPLv3. SQLite no tiene restricciones.
- qm-dsp no tiene CMake: los fuentes y las definiciones
  (`kiss_fft_scalar=double`) se declaran a mano en `cmake/QmDsp.cmake`.
  Al actualizar el commit hay que revisar que la lista siga completa.
- Los avisos de qm-dsp, SQLite y TagLib no se propagan: sus targets llevan
  sus propias opciones de advertencias.
- Riesgo abierto: el build en MSVC de qm-dsp y TagLib se verifica en CI
  (Windows), no en el Mac de desarrollo.
