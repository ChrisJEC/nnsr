# Castle color

4 imágenes LR de 128×128. Caso `-m 2` predefinido.

Sin vectores predefinidos (calculados):

```bash
./bin/nnsr -p images/LR/castle_color/ -o ImageResults/castle/calc/ -E -G
```

Con vectores predefinidos (caso 2):

```bash
./bin/nnsr -p images/LR/castle_color/ -o ImageResults/castle/no_calc/ -E -G -m 2
```

Notas:
- Factor por defecto (2), salida 256×256.
- Corridas anteriores no usaron `-b 1`.
