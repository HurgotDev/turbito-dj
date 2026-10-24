# 0001 – Licencia del proyecto: AGPLv3

Oct 24, 2026 · Aceptada

## Contexto

Turbito DJ es un proyecto personal, sin fines de lucro, con repositorio
público en GitHub para que cualquiera pueda clonarlo, usarlo y colaborar.

Las dependencias principales son copyleft o tienen licencia doble:

- JUCE 9 (mismos términos que JUCE 8): AGPLv3 o licencia comercial.
- Rubber Band: GPL o licencia comercial.
- Essentia: AGPLv3 o licencia propietaria.
- aubio: GPLv3.
- SQLite (dominio público), Catch2 (BSL-1.0) y las fuentes (OFL): sin
  restricciones de copyleft.

La licencia MIT inicial no es compatible: una app que enlaza JUCE bajo AGPLv3
tiene que distribuirse completa bajo AGPLv3.

## Decisión

El proyecto se publica bajo la **GNU Affero General Public License v3.0**
(`LICENSE`), usando JUCE, Rubber Band, Essentia y aubio bajo sus licencias
libres. No se compra ninguna licencia comercial.

## Consecuencias

- Costo 0 y sin cambios en el stack técnico.
- Quien distribuya la app o una versión modificada debe publicar su código
  bajo AGPLv3.
- Toda dependencia nueva debe ser compatible con AGPLv3 (GPLv3, LGPL, MIT,
  BSD, Apache-2.0, dominio público…). Nada propietario ni solo comercial.
- La app no puede publicarse en la Mac App Store (incompatible con GPL); se
  distribuye como descarga firmada y notarizada.
- Rubber Band se usa bajo su opción "GPL v2 o posterior", compatible con
  AGPLv3; confirmarlo al integrarlo en la fase 6.
- El SDK de ASIO (Windows) tiene licencia propia de Steinberg; revisar su
  compatibilidad con AGPLv3 antes de habilitar ASIO.
- Si en el futuro se quisiera vender con código cerrado, habría que comprar
  las licencias comerciales o reemplazar las dependencias copyleft.
