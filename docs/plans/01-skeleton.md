# Fase 1 – Esqueleto (plan)

## Contexto

El repo solo tiene especificación, diseño, assets y audios de prueba; todavía
no hay código. La fase 1 del spec (`docs/SPEC.md`, "Plan de fases") pide:
proyecto CMake con JUCE, ventana vacía de 1440 × 900 y CI en GitHub Actions
para macOS y Windows. `CLAUDE.md` ya existe. Al terminar la fase, el proyecto
debe compilar en las dos plataformas con las pruebas en verde, como exige la
convención de cierre de fase.

Decisiones tomadas:
- **JUCE 9.0.4** en vez de JUCE 8: tiene la misma licencia AGPLv3 y soporta
  fuentes variables.
- Dependencias con `FetchContent` y versión fija.
- Catch2 **v3.16.1**.
- El plan de la fase va en `docs/plans/01-skeleton.md` (convención de
  `CLAUDE.md`) y no en `tasks/plan.md` + `tasks/todo.md` como sugiere la
  skill. Así queda un solo archivo por fase, con la checklist adentro.

En la máquina local no hay `cmake`: el primer paso es `brew install cmake`
(4.4.4). Xcode 27.1 y Apple clang 21 ya están instalados.

## Grafo de dependencias

```
T1 build + ventana ──► T2 pruebas (CTest) ──► T3 CI ──► T4 docs + commit
```

## Tareas (rebanadas verticales)

### T1 – App que abre la ventana (macOS local)
Archivos nuevos:
- `CMakeLists.txt`: `cmake_minimum_required(3.22)`, `project(TurbitoDJ
  VERSION 0.1.0)`, C++20, `CMAKE_OSX_DEPLOYMENT_TARGET 13.0`,
  `include(cmake/Dependencies.cmake)`, `add_subdirectory(src)` y, si
  `BUILD_TESTING` está activo, `add_subdirectory(tests)`.
- `cmake/Dependencies.cmake`: `FetchContent` de JUCE `9.0.4` y Catch2
  `v3.16.1`, con `GIT_SHALLOW`.
- `CMakePresets.json`:
  - configure: `macos-debug` (arquitectura nativa), `macos-release`
    (`arm64;x86_64`), `windows` (sin generador fijo: CMake elige el Visual
    Studio instalado; x64, multi-config), todos con
    `binaryDir build/${presetName}`;
  - build y test: `macos-debug`, `macos-release`, `windows-debug`,
    `windows-release`.
- `src/CMakeLists.txt`:
  - `juce_add_gui_app(TurbitoDJ PRODUCT_NAME "Turbito DJ" ICON_BIG
    assets/brand/turbito-icon-1024.png ICON_SMALL turbito-icon-256.png
    BUNDLE_ID com.hurgotdev.turbitodj)`;
  - definiciones `JUCE_WEB_BROWSER=0`, `JUCE_USE_CURL=0`;
  - enlaza `juce::juce_gui_basics` más los flags recomendados de JUCE.
- `src/app/Main.cpp`: `JUCEApplication` con `MainWindow`
  (`DocumentWindow`, barra de título nativa, redimensionable, tamaño mínimo
  y centrada a 1440 × 900 desde `Tokens.h`).
- `src/app/MainComponent.h/.cpp`: un `Component` que pinta el fondo con
  `tokens::bg`.
- `src/ui/look/Tokens.h`: solo `windowReference`, `windowMinimum` y `bg`,
  escritos a mano. Lleva el comentario `shortcut: hand-copied from
  design/tokens.json, generate from JSON in phase 4`.

Criterios de aceptación:
- `cmake --preset macos-debug && cmake --build --preset macos-debug`
  compila sin warnings nuevos.
- La app abre una ventana "Turbito DJ" de 1440 × 900 con fondo `#0A0A12`,
  no se puede achicar a menos de 1280 × 720 y muestra el ícono de
  `assets/brand/`.

### T2 – Prueba que conecta CTest y protege los tokens
- `tests/CMakeLists.txt`: ejecutable `turbito_tests` (Catch2WithMain +
  `juce::juce_core`), `catch_discover_tests`, y la ruta del repo pasada como
  `TURBITO_SOURCE_DIR`.
- `tests/ui/look/TokensTests.cpp`: lee `design/tokens.json` con
  `juce::JSON` y comprueba que `window.reference`, `window.minimum` y
  `color.bg` coinciden con `Tokens.h`. Así una copia desactualizada rompe el
  build.

Criterios de aceptación:
- `ctest --preset macos-debug` pasa.
- Si se cambia un valor en `Tokens.h`, la prueba falla.

### T3 – CI en GitHub Actions
- `.github/workflows/ci.yml`: corre en `push` y `pull_request`, con una
  matriz de `macos-latest` y `windows-latest`. Pasos: checkout, configure,
  build y test con los presets `macos-release` y `windows` + `windows-release`.
  Usa `ctest --output-on-failure`.

Criterios de aceptación:
- Los dos jobs quedan en verde en GitHub.
- El binario de macOS es Universal: `lipo -archs` muestra `x86_64 arm64`.

### T4 – Documentación y cierre
- `docs/plans/01-skeleton.md`: este plan, con la checklist de tareas.
- `README.md`: actualizar el estado (fase 1 lista) y la sección "Compilar"
  con los requisitos reales y los comandos de los presets.
- `docs/SPEC.md`, `CLAUDE.md` y `README.md`: cambiar "JUCE 8" por "JUCE 9".
- Commit y push a `main` solo después de confirmarlo con el usuario.

## Checkpoints
- Después de T1: revisar con el usuario la ventana abierta (captura).
- Después de T3: los dos jobs de CI en verde antes de cerrar la fase.

## Checklist

- [x] T1 – App que abre la ventana (macOS local)
- [x] Checkpoint: captura de la ventana revisada
- [x] T2 – Prueba de tokens conectada a CTest
- [x] T3 – CI en verde en macOS y Windows, binario Universal (run
  40111104964: Windows 6 min, macOS 35 min)
- [x] T4 – README, SPEC y CLAUDE.md actualizados (JUCE 9, Compilar)
- [x] Commit y push (después, la CI pasó a "Build & Test" + "Release"; ver
  PR #1)

## Lo que no entra en la fase 1
- Fuentes como `BinaryData` y `TurbitoLookAndFeel`: fase 4.
- `src/core` y `src/engine`: fase 2.
- Caché de dependencias en CI.
- Firma y notarización: fase 13.

## Verificación
1. `brew install cmake`
2. `cmake --preset macos-debug && cmake --build --preset macos-debug &&
   ctest --preset macos-debug`
3. Abrir `build/macos-debug/src/TurbitoDJ_artefacts/Debug/Turbito DJ.app`,
   tomar una captura y comprobar el tamaño, el color y el ícono.
4. Build de `macos-release` y `lipo -archs` del binario.
5. Push y `gh run watch`: los dos jobs en verde.
