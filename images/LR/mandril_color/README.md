# Mandril color

4 imágenes LR de 128×128. Caso `-m 4` predefinido.

Sin vectores predefinidos (calculados):

```bash
./bin/nnsr -p images/LR/mandril_color/ -o ImageResults/mandril/calc/ -E -G
```

Con vectores predefinidos (caso 4):

```bash
./bin/nnsr -p images/LR/mandril_color/ -o ImageResults/mandril/no_calc/ -E -G -m 4
```

Notas:
- Factor por defecto (2). Para la comparación con el paper usar `mandril_color_128` (10 imágenes, factor 4).
