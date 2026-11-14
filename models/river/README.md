# Input model directory

Place one complete COLMAP sparse model in this directory before running the
application:

```text
models/river/
  cameras.txt
  images.txt
  points3D.txt
```

The binary COLMAP format is also supported:

```text
models/river/
  cameras.bin
  images.bin
  points3D.bin
```

Do not mix stale BIN files with newly generated TXT files. When both formats
are present, the application loads the binary model first.

Generated files such as `mesh.ply` remain local and are not tracked by Git.

