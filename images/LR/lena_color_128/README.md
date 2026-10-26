# Lena color 128

8 imágenes LR de 128×128 de la tesis (experimento de Lenna, Tablas 1 y 2 del paper). Sin caso `-m`.

Comando (parámetros originales de la tesis, 8 imágenes):

```bash
./bin/nnsr -p images/LR/lena_color_128/ -o ImageResults/lena128/calc/ -f 4 -n 8 -z 0.001 -b 1 -G -E
```

Con 4 imágenes:

```bash
./bin/nnsr -p images/LR/lena_color_128/ -o ImageResults/_validated/lenna_4img/ -f 4 -n 4 -z 0.001 -b 1 -G -E
```

Notas:
- Factor 4: salida 512×512, comparable con `lenna.png` en `images/originals_HR/`.
- NCC contra el original: 8 imágenes 0.996694 (publicado 0.996695); 4 imágenes 0.996598 (publicado 0.9966).
- Estas imágenes son las de la tesis. Las que estaban antes en el repo están en `images/backup/`.
