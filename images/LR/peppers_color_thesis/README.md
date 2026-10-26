# Peppers color (tesis)

8 imágenes LR de 128×128 de la tesis. Sin caso `-m` predefinido.

Comando (parámetros originales de la tesis, 8 imágenes):

```bash
./bin/nnsr -p images/LR/peppers_color_thesis/ -o ImageResults/_validated/peppers_8img/ -f 4 -n 8 -z 0.001 -b 1 -G -E
```

Notas:
- Factor 4: salida 512×512, comparable con `peppers.bmp` en `images/originals_HR/`.
- NCC contra el original: 0.960485. No está publicado en el paper.
