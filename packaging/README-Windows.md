# SF-Recon Meshing for Windows

This package contains a prebuilt Windows x64 version of `FeedbackRTSfM.exe`.

## Run

1. Keep the EXE, DLL files, and plugin directories together.
2. Put a complete COLMAP TXT or BIN model in `../models/river`, or edit
   `settings.ini` to point to another model directory.
3. Start `FeedbackRTSfM.exe`.
4. Click **Load Model**.
5. Click **Calculate Mesh**.

The default output is `../models/river/mesh.ply`.

If TXT and BIN files are both present, the application loads the BIN files
first. Remove stale BIN files when testing newly generated TXT tracks.

## Source and licenses

The corresponding application source is provided in the GitHub repository from
which this release was downloaded. The application is licensed under the GNU
General Public License v3.0 or later. See `LICENSE` and
`THIRD_PARTY_NOTICES.md`.

The executable is currently unsigned, so Windows may display an unknown
publisher warning.

