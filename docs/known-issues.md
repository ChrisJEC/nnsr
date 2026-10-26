# Known Issues

## Crash on `lena_color_64` with `-m 3` (predefined motion vectors)

**Status:** Unresolved — root cause identified, workaround not found via CLI flags.

### Reproduction

```bash
./bin/nnsr -p images/LR/lena_color_64/ -o ImageResults/lena64/no_calc/ -E -G -m 3
```

Output:

```
nnsr: delaunay.c:201: tio2delaunay: Assertion `status' failed.
Aborted
```

### Root cause

The crash happens inside the `nn` (Natural Neighbor) dependency library, not
in this project's own code.

During Delaunay triangulation, `tio2delaunay()` calls `circle_build1()` for
every triangle to compute its circumcircle:

```c
// nn-c/nn/nncommon-vulnerable.c
int circle_build1(circle* c, point* p1, point* p2, point* p3)
{
    ...
    double denom = x2 * y3 - y2 * x3;   // twice the triangle's signed area
    if (denom == 0.0) {                 // the 3 points are exactly collinear
        c->x = NaN; c->y = NaN; c->r = NaN;
        return 0;
    }
    ...
}
```

```c
// nn-c/nn/delaunay.c:200-201
status = circle_build1(c, &d->points[t->vids[0]], &d->points[t->vids[1]], &d->points[t->vids[2]]);
assert(status);   // <-- aborts here when denom == 0.0
```

`Triangle` hands back a triangle whose three vertices are **exactly
collinear** after the low-resolution points are mapped onto the HR grid with
the hardcoded motion vectors for case 3 (`-m 3`, `src/main.c:703-709`). The
file is literally named `nncommon-vulnerable.c` by the library author,
acknowledging it is sensitive to this kind of floating-point degeneracy.

### Why `-z <ZeroTolerance>` does NOT fix it

`ZeroTolerance` is only applied inside `Motion_Estimation()`
(`src/main.c:623-635`) to the **automatically computed** vectors. The
predefined `-m` vectors are written directly into `Motion_Vectors` inside
`HR_grid_map()` (`src/main.c:684` onward), independently of
`Motion_Estimation()` — the zero-angle correction never touches the `-m`
path. Confirmed by testing `-z 0.0001` through `-z 0.02`: no effect, same crash.

### What does work

Running the same dataset **without** `-m` (auto-computed vectors) succeeds
cleanly (0.01% corrupted pixels with the current defaults):

```bash
./bin/nnsr -p images/LR/lena_color_64/ -o ImageResults/lena64/calc/ -E -G
```

### Possible fixes (not applied — would require modifying code)

1. Recalibrate the hardcoded `Motion_Vectors` for case 3 in `src/main.c`.
2. Patch `nn-c/nn/delaunay.c` to handle a zero-area triangle instead of
   asserting.
3. Add a tiny epsilon jitter to near-collinear mapped points before
   triangulation.
