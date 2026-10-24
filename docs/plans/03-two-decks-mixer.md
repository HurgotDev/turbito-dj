# Fase 3 – Dos decks + mezclador (plan)

## Contexto

La fase 2 dejó un deck (A) con play, pausa y CUE sobre `AudioEngine`. La
fase 3 del spec pide: **dos decks + mezclador: gain, EQ, filtro, faders,
crossfader y vúmetros, con pruebas unitarias.**

El mockup (`design/mockup-reference.html`) fija los controles por canal:
knobs Gain, Agudos, Medios, Graves y Filtro, un fader vertical y un vúmetro
de 14 segmentos que sube y baja con el fader (post-fader). Abajo, un
crossfader. No hay knob de master, vúmetro de master ni botones de kill: el
kill es el knob de EQ al mínimo. La curva del crossfader no está en el
mockup; vive como parámetro del motor y en la UI provisional.

Decisiones de diseño:
- **Todo en el hilo de audio sin reservar memoria**: buffers de trabajo de
  tamaño fijo (`maxBlock = 1024` muestras por canal y deck) como miembros;
  el callback procesa en trozos de hasta `maxBlock`, así que cualquier
  tamaño de buffer funciona sin reservar.
- **Filtros con `juce::IIRCoefficients` + `juce::IIRFilter`**
  (`juce_audio_basics`, ya enlazado). Se usa
  `processSingleSampleRaw` por muestra: no toma el `SpinLock` interno. Sin
  `juce_dsp` (sus `Coefficients` son ref-counted y reservan memoria).
- **EQ tipo isolator con kill real**: cruces Linkwitz-Riley de 4.º orden a
  250 Hz y 2.5 kHz (dos biquads Butterworth en cascada, Q = 0.7071).
  - graves = AP₂(LP₁(x)), medios = LP₂(HP₁(x)), agudos = HP₂(HP₁(x)),
    donde AP₂ es un pasa-todo de 2.º orden a 2.5 kHz (Q = 0.7071) que
    alinea la fase de los graves.
  - Con las tres bandas a 0 dB la suma es un pasa-todo: magnitud plana.
  - Kill de una banda = ganancia 0: atenúa con pendiente de 24 dB/oct.
- **Mapas de los knobs** (valor normalizado 0–1 de la capa de acciones;
  centro 0.5 = neutro, doble clic vuelve al valor por defecto):
  - Gain: −12 a +12 dB lineal en dB; por defecto 0.5 (0 dB).
  - EQ: `v ≤ 0.5 → (2v)²` (0 = kill), `v > 0.5 → +12·(v − 0.5) dB`
    (máx. +6 dB); por defecto 0.5.
  - Filtro: un knob, centro = sin filtro (zona muerta ±0.02). Hacia la
    izquierda pasa-bajos de 20 kHz a 80 Hz; hacia la derecha pasa-altos de
    20 Hz a 8 kHz; barrido exponencial, Q = 1. Por defecto 0.5.
  - Fader: ganancia `v²`; por defecto 1 en la UI provisional.
- **Crossfader**: posición `x` (0 = todo A) y curva `c` (0–1):
  `s = 16^c`, `gA = sin(π/2 · min(1, (1 − x)·s))`,
  `gB = sin(π/2 · min(1, x·s))`.
  - `c = 0`: potencia constante (suave, por defecto); `c = 1`: corte en el
    6 % final del recorrido (scratch).
  - Función pura en `engine/mixer/Crossfader.h`, fácil de probar.
- **Sin chasquidos al mover controles**: ganancias con
  `juce::SmoothedValue` (rampa de 20 ms); el corte del filtro se suaviza y
  sus coeficientes se recalculan cada 32 muestras.
- **Vúmetro**: pico absoluto por bloque, post-fader y pre-crossfader, en un
  atomic del snapshot. La UI aplica caída y retención (decay) por su cuenta.
- **Sin limitador de master**: dos pistas a 0 dBFS pueden saturar; se
  controla con gain y faders, como en un mezclador físico.

## Diseño

```
UI ──ActionEvent──► SpscQueue ──► AudioEngine (hilo de audio)
UI ──{deck, Track*}──► SpscQueue(load) ──► Deck A | Deck B
                                             │       │
                                     Channel A   Channel B   (gain, EQ,
                                             │       │        filtro, fader)
                                             └─ Crossfader ─► salida 0/1
UI ◄── EngineSnapshot { DeckSnapshot decks[2] } (+ channelPeak) ◄── motor
```

- `core/Action.h`: se agregan `DeckB_Play`, `DeckB_Cue`,
  `ChannelA_Gain`, `ChannelA_EqHigh`, `ChannelA_EqMid`, `ChannelA_EqLow`,
  `ChannelA_Filter`, `ChannelA_Fader`, lo mismo para B, `Mixer_Crossfader`
  y `Mixer_CrossfaderCurve`. Valores 0–1.
- `core/DeckSnapshot.h`: agrega `std::atomic<float> channelPeak`.
  `core/EngineSnapshot.h`: `DeckSnapshot decks[2]`.
- `engine/deck/Deck`: `handle(ActionEvent)` pasa a `play(bool pressed)` y
  `cue(bool pressed)`; el motor enruta por deck. Mismo comportamiento.
- `engine/mixer/Channel.h/.cpp`: `prepare(sampleRate)`, un setter por
  control (`setGain`, `setEqHigh`, `setEqMid`, `setEqLow`, `setFilter`,
  `setFader`, valores 0–1; no conoce `Action`),
  `process(float* left, float* right, int numSamples)` en sitio, y
  `peak()` del último bloque. Sin reservas.
- `engine/mixer/Crossfader.h`: `CrossfaderGains crossfaderGains(float x,
  float curve)`.
- `AudioEngine`: dos `Deck`, dos `Channel`, buffers fijos; por trozo limpia
  los buffers, renderiza cada deck en el suyo, procesa su canal y suma a la
  salida con las ganancias del crossfader (suavizadas). Salida mono: solo
  canal izquierdo; más de 2 salidas: el resto en silencio.
  `load(int deck, std::unique_ptr<Track>)`.

## Tareas

### T1 – Core: acciones y snapshots
- `Action`, `DeckSnapshot.channelPeak`, `EngineSnapshot`.

### T2 – Engine: `Crossfader` y `Channel`
- `tests/engine/mixer/CrossfaderTests.cpp`: extremos (1,0) y (0,1);
  `c = 0` → `gA² + gB² ≈ 1` en todo el recorrido; `c = 1` → ambos 1 en el
  centro; ganancias monótonas.
- `tests/engine/mixer/ChannelTests.cpp` (senos sintéticos a 44.1 kHz,
  medir RMS tras 0.2 s de asentamiento):
  - Todo por defecto: 50 Hz, 1 kHz y 10 kHz pasan a ±0.1 dB.
  - Kill de graves: 50 Hz cae > 40 dB; kill de agudos: 10 kHz cae > 40 dB;
    kill de medios: 1 kHz cae > 20 dB, y en cada caso las otras bandas
    pasan a ±1 dB.
  - EQ al máximo: +6 dB ±0.5 en el centro de su banda.
  - Gain 1 → +12 dB; 0 → −12 dB. Fader 0 → silencio.
  - Filtro 0.1 (LPF): 8 kHz cae > 20 dB, 100 Hz pasa a ±1 dB; filtro 0.9
    (HPF): 100 Hz cae > 20 dB, 8 kHz pasa a ±1 dB; 0.5 → sin cambio.
  - `peak()` refleja el fader.

### T3 – Engine: dos decks en `AudioEngine`
- `Deck` con `play`/`cue`; `DeckTests` actualizadas (mismos casos).
- `AudioEngineTests`: cada acción llega a su deck; cargar en B retira la
  pista vieja de B; crossfader en 0 → solo suena A y en 1 → solo B (pistas
  constantes distintas); un bloque de 3000 muestras (> `maxBlock`) avanza
  3000; `channelPeak` del snapshot sigue al fader.

### T4 – App: UI provisional de dos decks y mezclador
- Una fila por deck: Cargar, Play, CUE, tiempo y nombre (como en fase 2).
- Mezclador con `juce::Slider` por canal: rotarios Gain, Agudos, Medios,
  Graves, Filtro; fader vertical; barra simple de vúmetro pintada con
  `vuLow`/`vuMid`/`vuHigh`. Crossfader horizontal y slider "Curva".
  Doble clic devuelve el valor por defecto (`setDoubleClickReturnValue`).
- Atajos del spec: `Espacio`/`Q` deck A, `Enter`/`P` deck B.
- Textos nuevos en `src/ui/Strings.h`.
- Aceptación manual: dos pistas sonando a la vez, crossfader y EQ audibles
  sin chasquidos, kill de graves quita el bombo.

### T5 – Documentación y cierre
- `README.md` (estado: fase 3 lista), `CHANGELOG.md` `[Unreleased]`, esta
  checklist. Push y PR a `develop`; el usuario mergea.

## Checkpoints
- Después de T4: el usuario prueba la app con audio real.
- "Build & Test" en verde en macOS y Windows antes de pedir el merge.

## Lo que no entra
- UI fiel al mockup (`Knob`, `VuMeter`, 14 segmentos): fase 4.
- Pitch, keylock, sync: fase 6. Efectos: fase 8. Bus de placas: fase 9.
- Atajos de crossfader (`←`/`→`) y mapeo de ratón completo: fase 11.
- Guardar la curva del crossfader en ajustes: cuando exista la pantalla
  de Ajustes (fase 4 o posterior).
- Pre-escucha por auriculares (cue de cabina) y limitador de master.

## Verificación
1. `cmake --preset macos-debug && cmake --build --preset macos-debug &&
   ctest --preset macos-debug` en verde.
2. Abrir la app, cargar `beat_124bpm_30s.wav` en A y `beat_126bpm_30s.wav`
   en B, sonar ambas, mover crossfader, faders, EQ y filtro.
3. Push y `gh run watch`: macOS y Windows en verde.

## Checklist

- [x] T1 – Core: acciones y snapshots
- [x] T2 – Engine: `Crossfader` y `Channel`
- [x] T3 – Engine: dos decks en `AudioEngine`
- [x] T4 – App: UI provisional de dos decks y mezclador
- [ ] Checkpoint: el usuario prueba la app con audio real
- [x] T5 – README, CHANGELOG y PR a `develop`
- [ ] "Build & Test" en verde en macOS y Windows
