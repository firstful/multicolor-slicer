# multicolor-slicer

A headless, multi-color FDM slicer built from a heavily pruned subset of
[BambuStudio](https://github.com/bambulab/BambuStudio) (AGPL-3.0).

It exists because Bambu Studio's own CLI cannot slice a multi-color (AMS) project
correctly: the CLI path force-resets `has_filament_switcher` to false, which
collapses the project to a single extruder and silently drops every filament
change. This tree removes the GUI, removes the CLI override, and keeps the
slicing engine intact.

## What it does

```
STL / 3MF  ->  multicolor-slicer  ->  .gcode  ->  Bambu P1S (AMS)
```

* Reads a Bambu project (`.3mf`) or a plain model.
* Applies the plate configuration and the per-object filament map.
* Slices and exports G-code with correct AMS tool changes.

For a Bambu P1S the AMS change is emitted as `M625` + `M1020 S<n> H0`, not a
bare `T`/`T1`. `S0` is the first filament slot, `S1` the second, and so on.

## Building

The build uses **system dependencies only** — no vendored dependency tree is
compiled. On Debian/Ubuntu:

```bash
sudo apt install -y \
    build-essential cmake ninja-build pkg-config \
    libboost-all-dev libtbb-dev libcereal-dev libeigen3-dev \
    libnlopt-dev libssh2-dev libassimp-dev \
    libssl-dev zlib1g-dev libexpat1-dev libpng-dev \
    libfreetype6-dev libfontconfig1-dev libgl-dev \
    libnoise-dev libgl1-mesa-dev libglu1-mesa-dev

cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DSLIC3R_GUI=OFF \
    -DSLIC3R_ALLOW_MCUT_BOOLEAN=ON \
    -DBBL_RELEASE_TO_PUBLIC=1 \
    -DBBL_INTERNAL_TESTING=0
ninja -C build -j"$(nproc)"
```

The binary lands at `build/src/multicolor-slicer`.

> **Eigen version.** BambuStudio pins Eigen 3.4 and its `find_package` rejects
> anything newer. Distribution Eigen 3.4 packages are fine; if your distro ships
> Eigen 5.x, point CMake at a 3.4 header tree:
> `-DEIGEN3_INCLUDE_DIR=/path/to/eigen-3.4.0`.

> **libnoise.** BambuStudio expects a `libnoise_static` target. If your distro
> only ships `libnoise.so`, pass `-DLIBNOISE_INCLUDE_DIR=/usr/include/noise
> -DLIBNOISE_LIBRARY=/usr/lib/libnoise.so`.

## Usage

```bash
./build/src/multicolor-slicer --slice 0 --export-3mf out.3mf in.3mf
./build/src/multicolor-slicer --slice 0 --export-gcode out.gcode in.3mf
```

## What was pruned

This is not a stock BambuStudio build. The following were removed to cut build
time, dependency count and peak memory:

| Removed | Reason |
| --- | --- |
| GUI (`src/slic3r`, imgui, imguizmo, hidapi) | headless CLI only |
| OpenCV | textured OBJ import / texture painting, not needed for STL/3MF |
| CGAL | cut / boolean / emboss / triangulation features only |
| OpenCascade (OCCT), STEP | CAD import only |
| assimp | glTF/FBX import only |
| OpenVDB | volume-based tree support only |
| Boost.Geometry, Boost.Log core, Boost.Locale, Boost.DateTime, Boost.Thread | each used by a handful of files; included directly by their users instead |
| Tree (organic) support | requires OpenVDB, which is not in the lean dependency set |
| Resin printing (SLA, SL1, `Format/SL1`) | not FDM |
| AMF, SVG, CSGMesh, `Shape/TextShape`, `Win10ModelRepair`, emboss, Voronoi | legacy / non-FDM feature paths |
| `libslic3r_gui` | GUI only |

Two replacements were written rather than vendored:

* **`libslic3r/Triangulation.cpp`** — was CGAL's constrained Delaunay
  triangulation, now the Mapbox [earcut](https://github.com/mapbox/earcut)
  triangulator that is already vendored in this tree. Same contract: triangle
  indices address the input polygon's points and keep the input's winding.
* **No precompiled header required.** The PCH force-included ~40 third-party
  headers into every translation unit, so each `cc1plus` process held the whole
  Boost/Eigen/TBB AST even when the TU used none of it. The PCH now carries
  only the standard headers plus the libslic3r core headers, and every user
  includes what it needs directly. `SLIC3R_PCH=OFF` is fully supported.

## License

AGPL-3.0, inherited from BambuStudio. See `LICENSE`.
