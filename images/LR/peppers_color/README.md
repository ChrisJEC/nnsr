# Peppers color

4 imágenes LR de 128×128. Caso `-m 5` predefinido.

Sin vectores predefinidos (calculados):

```bash
./bin/nnsr -p images/LR/peppers_color/ -o ImageResults/peppers/calc/ -E -G
```

Con vectores predefinidos (caso 5):

```bash
./bin/nnsr -p images/LR/peppers_color/ -o ImageResults/peppers/no_calc/ -E -G -m 5
```

Notas:
- Factor por defecto (2). Para la versión de la tesis (8 imágenes) usar `peppers_color_thesis`.
