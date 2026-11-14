# Model directory

Place one COLMAP sparse model in a subdirectory such as `models/river`.
The application accepts either a complete TXT model:

```text
cameras.txt
images.txt
points3D.txt
```

or a complete BIN model:

```text
cameras.bin
images.bin
points3D.bin
```

If both representations are present, the BIN model is loaded first. Generated
meshes and local datasets are intentionally excluded from version control.
