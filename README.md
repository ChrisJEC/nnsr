# Natural Neighbor Super-Resolution (nnsr)

This project implements a **Super-Resolution Image Reconstruction** algorithm
based on **Natural Neighbor Interpolation**, originally developed as part of
a thesis in **2012**, and later published in the paper
[Super-Resolution Image Reconstruction using Natural Neighbor Interpolation](https://www.scielo.org.mx/scielo.php?script=sci_arttext&pid=S1405-55462015000200002)
(*Computación y Sistemas*, Vol. 19, No. 2, 2015 — also included as
[`docs/v19n2a2.pdf`](docs/v19n2a2.pdf)).

From multiple low-resolution (LR) images of the same scene with sub-pixel
shifts between them, the algorithm estimates motion between frames (via an
FFT-based method), maps every LR pixel onto a shared high-resolution (HR)
grid, and interpolates the missing values using Natural Neighbor
interpolation (Sibson's method) for the luminance channel, with bilinear
interpolation for chrominance.

⚠️ **Note:** This code was written in **2012** and does **not** reflect
current programming style or best practices.
⚠️ **Important:** This software is only compatible with **version 1.82** of
the `nn` library, specifically commit
[`32fab25e186bbe511bb5aea17b164377d853f680`](https://github.com/sakov/nn-c/commit/32fab25e186bbe511bb5aea17b164377d853f680).

---

## Features

- Super-resolution of images using Natural Neighbor Interpolation.
- Automatic (FFT-based) or predefined motion-vector estimation between LR frames.
- Support for color and grayscale (Y/Cb/Cr) output.
- Optional edge extrapolation and border handling for the HR grid.

---

## Project Structure

```
nnsr/
├── src/main.c              # Source code
├── include/nnsr.h          # Header / configuration constants
├── Makefile                # Builds ./bin/nnsr
├── docs/
│   ├── v19n2a2.pdf          # The published paper
│   └── known-issues.md      # Known bugs / limitations
├── images/
│   ├── LR/                  # Low-resolution test datasets, one folder each,
│   │   └── <dataset>/README.md   #   with the exact command to reconstruct it
│   └── originals_HR/        # True high-resolution references (for validation)
├── LICENSE
└── README.md
```

`bin/` (compiled binary) and `ImageResults/` (generated reconstructions) are
build artifacts — not tracked in git. `ImageResults/` is fully regenerable
by running the commands documented in each `images/LR/<dataset>/README.md`.

---

## Dependencies

- `libfftw3` — Fast Fourier Transform library (GPL v2 or later).
- `libjpeg` — JPEG image reading.
- `libm` — standard math library.
- [`nn`](https://github.com/sakov/nn-c) — Natural Neighbor Interpolation
  library, version **1.82**, commit
  [`32fab25e186bbe511bb5aea17b164377d853f680`](https://github.com/sakov/nn-c/commit/32fab25e186bbe511bb5aea17b164377d853f680).
  Must be built and installed separately (`libnn`) before compiling this
  project.

On Debian/Ubuntu, the first three can be installed with:

```bash
sudo apt install libfftw3-dev libjpeg-dev
```

---

## Compilation

From the repository root:

```bash
make clean
make
```

This produces `./bin/nnsr`.

---

## Running

Basic example:

```bash
./bin/nnsr -p images/LR/castle_color/ -o ImageResults/castle/calc/ -E -G
```

Every dataset under `images/LR/` has its own `README.md` with the exact
command (including the right `-f`, `-n`, `-m`, `-z` values) used to validate
that dataset — start there instead of guessing parameters.

### Parameters

| Flag | Meaning |
|---|---|
| `-p <path>` | Path to the LR input images directory (required) |
| `-o <path>` | Path to write the HR output images (required) |
| `-f <factor>` | Up-scale factor (default: 2) |
| `-n <NumLRimages>` | Number of LR images to use, 2-16 (default: 4) |
| `-m <TestMEVectors>` | Use predefined motion vectors instead of computing them (1-5; see `src/main.c` for which dataset each case was calibrated for). Omit to compute motion automatically. |
| `-z <ZeroTolerance>` | Rounds near-zero *computed* rotation angles to exactly zero (only affects auto-computed vectors, not `-m`). Default `0.001`; smaller LR images may need a larger value — see `docs/known-issues.md`. |
| `-b <EdgeBorderType>` | `0`: no border, `1`: duplicate edge pixels, `2`: fill edge border (default: 0) |
| `-G` | Also output the `Y`/`Cb`/`Cr` channel images, not just the final color image |
| `-E` | Enable edge extrapolation (recommended — without it, HR grid points outside the convex hull of mapped points are left undefined) |
| `-t <Thingrid>` | Thin input points by averaging within clusters (advanced, rarely needed) |
| `-v <verbose>` | Debug verbosity, 0-19 |
| `-H` | Full help message |

---

## Test datasets

`images/LR/` includes 11 datasets, four of which (`lena_color_128`,
`mandril_color_128`, `peppers_color_thesis`, `jellybeans_color`) come from
the original thesis project and can be validated against a true HR original
in `images/originals_HR/` (see each dataset's `README.md` for the resulting
NCC). The rest have no known HR original.

| Dataset | LR images | Size | Predefined `-m` |
|---|---|---|---|
| `castle_color` | 4 | 128×128 | `2` |
| `mandril_color` | 4 | 128×128 | `4` |
| `mandril_color_128` | 10 | 128×128 | — |
| `peppers_color` | 4 | 128×128 | `5` |
| `peppers_color32` | 4 | 32×32 | — |
| `peppers_color_thesis` | 8 | 128×128 | — |
| `lena_color_256` | 4 | 64×64 *(name is historical, not the actual size)* | `1` |
| `lena_color_64` | 4 | 64×64 | `3` *(crashes — see known issues)* |
| `lena_color_128` | 8 | 128×128 | — |
| `jellybeans_color` | 8 | 64×64 | — |
| `Adyoron` | 40 (only 16 usable, see `-n` limit) | 138×115 | — |

---

## Known issues

See [`docs/known-issues.md`](docs/known-issues.md) — currently one open
issue: `lena_color_64` with `-m 3` crashes inside the `nn` library's
Delaunay triangulation (predefined vectors only; auto-computed vectors for
the same dataset work fine).

---

## License

This project's own code is distributed under the **GNU General Public
License v2**. **However**, because it links against the `nn` library (which
compiles Jonathan Shewchuk's Triangle), **the compiled binary as a whole
cannot be used, sold, or distributed as part of a commercial product**
without separate permission from Shewchuk — non-commercial, research, and
educational use is unaffected. See [`LICENSE`](LICENSE) for the full text
and third-party notices (FFTW3, libjpeg, `nn`, Triangle).

---

## Reference

If you use this software in academic work, please cite:

> Enríquez-Cervantes, C. J. & Rodríguez-Dagnino, R. M. (2015). **A
> Super-Resolution Image Reconstruction using Natural Neighbor
> Interpolation**. *Computación y Sistemas*, 19(2), 211-231.
> https://doi.org/10.13053/CyS-19-2-2068

---

## Contact

For questions about this project, contact Christian Enriquez —
cjenriquezc@gmail.com.
