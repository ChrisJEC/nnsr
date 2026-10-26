# Jelly Beans color

8 imágenes LR de 64×64 de la tesis (experimento de Jelly Beans, Tablas 5 y 6 del paper). Sin caso `-m`.

Comando (parámetros originales de la tesis):

```bash
./bin/nnsr -p images/LR/jellybeans_color/ -o ImageResults/_validated/jellybeans_8img/ -f 4 -n 8 -z 0.001 -b 1 -G -E
```

Notas:
- Factor 4: salida 256×256, igual que el original `Jelly_Beans.tiff` en `images/originals_HR/`.
- NCC contra el original: 0.986394 (publicado: 0.986403).
