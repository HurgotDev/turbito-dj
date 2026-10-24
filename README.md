# Kit de arranque de Turbito DJ

Copia el contenido de esta carpeta en la raíz del repositorio nuevo.

```
CLAUDE.md                     Reglas del proyecto para Claude Code
docs/SPEC.md                  ← AGREGAR: exporta el doc "Turbito DJ – Especificación v1" a Markdown
design/mockup-reference.html  Fuente del mockup aprobado (referencia de verdad de la UI)
design/tokens.json            Colores, fuentes, radios y medidas
assets/brand/                 Logo, ícono (SVG + PNG de 16 a 1024 px) y versión horizontal
assets/fonts/                 Inter, JetBrains Mono, Titan One (licencia OFL incluida)
test-audio/                   Audios sintéticos con BPM, tono y silencios conocidos
```

Primera instrucción sugerida en Claude Code (en modo plan):

> Lee CLAUDE.md, docs/SPEC.md y design/tokens.json. Propón el plan de la fase 1 (esqueleto CMake + JUCE, ventana de 1440 × 900 con el ícono de assets/brand, CI para macOS y Windows) y espera mi aprobación.
