# Lena color 256

4 imágenes LR de **64×64** (el nombre dice 256, pero el tamaño real es 64). Caso `-m 1` predefinido.

Comando validado:

```bash
./bin/nnsr -p images/LR/lena_color_256/ -o ImageResults/lena256/no_calc/ -m 1 -f 4 -E -G
```

Notas:
- Factor 4 (64 → 256). Con factor 2 la salida queda en 128×128.
- Con vectores calculados (sin `-m`) y factor 4, la reconstrucción falla (~56% de píxeles sin cubrir), por eso solo existe la versión `no_calc`.
