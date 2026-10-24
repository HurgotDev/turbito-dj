# Audios de prueba (sintéticos, sin derechos)

Generados por código para tener una "verdad conocida" en las pruebas del análisis y del motor.
Todos son WAV estéreo, 44.1 kHz, 16 bits.

| Archivo | Para probar | Valor esperado |
| --- | --- | --- |
| `beat_124bpm_30s.wav` | Detección de BPM y beatgrid, deck A | 124.0 BPM, primer beat en 0.000 s |
| `beat_126bpm_30s.wav` | SYNC entre decks, Auto emparejar BPM | 126.0 BPM, primer beat en 0.000 s |
| `silencios_120bpm.wav` | "Limpiar silencios" del Auto Mix | 3.0 s de silencio al inicio, audio hasta 23.0 s, 4.0 s de silencio al final |
| `tono_la_menor_8A_120bpm.wav` | Detección de tono (Camelot) | La menor = **8A**, 120 BPM |
| `placa_air_horn.wav` | Sampler de placas (one-shot y gate) | 1.6 s, tres toques |

Para pruebas con música real, usa pistas propias o con licencia libre; no subas música con derechos al repositorio.
