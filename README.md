# SF-Recon Meshing

SF-Recon Meshing is the surface-reconstruction stage used after SF-Recon
Gaussian training and COLMAP track generation. It reads a sparse COLMAP model,
constructs a 3D Delaunay triangulation, optimizes the inside/outside labeling
with graph cut, and exports a PLY triangle mesh.

This repository contains the Windows/Qt source code. Local datasets, generated
meshes, build products, research PDFs, and the prebuilt runtime bundle are not
tracked.

## Download the Windows demo

Download the ready-to-run Windows x64 package from
[v0.1-prebuilt-demo](https://github.com/lzh282140127-cell/SF-Recon-Meshing/releases/tag/v0.1-prebuilt-demo).
The ZIP archive contains `FeedbackRTSfM.exe`, the required DLLs and Qt plugins,
an example `settings.ini`, license notices, and usage instructions.

Extract the complete archive before running the executable. Do not copy only
`FeedbackRTSfM.exe`, because the program requires the bundled runtime DLLs.

## Pipeline

```text
SF-Recon training
  -> downsample_gaussian_ply.py
  -> gaussians_tracks_with_depth.py
  -> cameras.txt + images.txt + points3D.txt
  -> SF-Recon Meshing
  -> mesh.ply
```

## Input

The application accepts a complete COLMAP sparse model in either text or binary
form:

```text
models/river/
  cameras.txt
  images.txt
  points3D.txt
```

or:

```text
models/river/
  cameras.bin
  images.bin
  points3D.bin
```

When both representations are present, the binary model is loaded first. Do not
leave stale BIN files beside newly generated TXT files.

## Windows quick start

1. Build the `Release | x64` configuration described below.
2. Create a `Release` directory at the repository root and copy the built
   `FeedbackRTSfM.exe` into it.
3. Copy `config/settings.example.ini` to `Release/settings.ini`.
4. Put the COLMAP model under `models/river`, or edit the paths in
   `settings.ini`.
5. Start `FeedbackRTSfM.exe` with `Release` as its working directory. Double
   clicking `Release/FeedbackRTSfM.exe` does this automatically.
6. Click **Load Model** and wait for the point cloud to appear.
7. Click **Calculate Mesh** and wait for reconstruction to finish.
8. Click **Show mesh** to inspect the result.

With the example configuration, the output is written to:

```text
models/river/mesh.ply
```

## Build from source

The checked-in Visual Studio project targets Windows x64 and C++17. The known
working environment is:

- Visual Studio 2019 with the Desktop development with C++ workload
- MSVC v142 and Windows 10 SDK
- Qt 5.12.12, MSVC 2017 64-bit, registered in Qt VS Tools as
  `5.12.12_msvc2017_64`
- vcpkg with x64-windows integration

Install the manifest dependencies from the repository root:

```powershell
vcpkg install --triplet x64-windows
vcpkg integrate install
```

Then open:

```text
FeedBackSfMPrj/FeedbackRTSfM.sln
```

Select `Release | x64` and build the `FeedbackRTSfM` project. The vcpkg
manifest installs CGAL, Boost, Ceres, Eigen, glog, METIS, TBB, and their
transitive dependencies. Qt is installed separately through Qt VS Tools.

To prepare the local runtime directory, copy the executable to `Release` and
run Qt's deployment tool:

```powershell
windeployqt Release\FeedbackRTSfM.exe
```

A redistributable binary archive must also include the license materials
required by every bundled dependency. Prebuilt binaries are intentionally kept
out of the source tree.

## Batch processing

The `scripts` directory contains Windows PowerShell automation for processing
multiple COLMAP model directories. Start with a single model through the GUI
before running the batch scripts.

## Method and acknowledgements

The meshing implementation follows:

> P. Labatut, J.-P. Pons, and R. Keriven. Robust and Efficient Surface
> Reconstruction from Range Data. Computer Graphics Forum, 2009.

Parts of the reconstruction data model, graph-cut utilities, and Delaunay
meshing implementation are derived from
[COLMAP](https://github.com/colmap/colmap). Geometry processing uses
[CGAL](https://www.cgal.org/). See `THIRD_PARTY_NOTICES.md` for attribution and
license details.

## License

This project is distributed under the GNU General Public License v3.0 or later
because it uses CGAL's GPL-licensed 3D Triangulations package. See `LICENSE`.
Third-party components remain under their respective licenses.


