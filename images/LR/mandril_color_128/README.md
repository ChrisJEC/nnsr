# Mandril color 128

10 imágenes LR de 128×128 de la tesis (experimento de Mandrill, Tablas 3 y 4 del paper). Sin caso `-m`.

Comando (parámetros originales de la tesis, 10 imágenes):

```bash
./bin/nnsr -p images/LR/mandril_color_128/ -o ImageResults/mandril128/calc/ -f 4 -n 10 -z 0.001 -b 1 -G -E
```

Con 4 imágenes (usa solo las imágenes 1 a 4 del set):

```bash
./bin/nnsr -p images/LR/mandril_color_128/ -o ImageResults/_validated/mandril_4img/ -f 4 -n 4 -z 0.001 -b 1 -G -E
```

Notas:
- Factor 4: salida 512×512, comparable con `mandril_color.tif` en `images/originals_HR/`.
- NCC contra el original: 10 imágenes 0.906779 (publicado 0.906983); 4 imágenes 0.891519 (publicado 0.891909).
