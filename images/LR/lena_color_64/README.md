# Lena color 64

4 imágenes LR de 64×64. Caso `-m 3` predefinido.

Sin vectores predefinidos (calculados):

```bash
./bin/nnsr -p images/LR/lena_color_64/ -o ImageResults/lena64/calc/ -E -G
```

Con vectores predefinidos (caso 3), **crashea**:

```bash
./bin/nnsr -p images/LR/lena_color_64/ -o ImageResults/lena64/no_calc/ -E -G -m 3
```

Error: `delaunay.c:201: tio2delaunay: Assertion 'status' failed.`. Ver `docs/known-issues.md`.

Notas:
- La carpeta tiene una subcarpeta `archive?` que no se usa.
