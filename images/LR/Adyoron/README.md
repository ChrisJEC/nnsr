# Adyoron

40 imágenes LR de 138×115 (no cuadradas). Sin caso `-m` predefinido.

Comando (desde la raíz del repo):

```bash
./bin/nnsr -p images/LR/Adyoron/ -o ImageResults/adyoron/calc/ -E -G -n 16
```

Notas:
- `-n` acepta máximo 16 (límite en `include/nnsr.h`), así que solo se usan las imágenes 1 a 16 de las 40.
- Vectores calculados automáticamente (no hay `-m`).
