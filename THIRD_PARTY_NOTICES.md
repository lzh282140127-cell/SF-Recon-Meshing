# Third-party notices

SF-Recon Meshing includes, derives from, links against, or is designed to be
built with the components listed below. Each component remains under its own
license. This document is not a substitute for the license text shipped by the
corresponding dependency.

## COLMAP

Parts of the camera model, reconstruction I/O, utility code, graph-cut code,
viewer resources, and Delaunay meshing implementation are derived from COLMAP.

Project: https://github.com/colmap/colmap  
License: BSD 3-Clause

Copyright (c) 2016, ETH Zurich and UNC Chapel Hill.  
Copyright (c) 2016-2026, The COLMAP Contributors.  
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of ETH Zurich and UNC Chapel Hill nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## CGAL

Project: https://www.cgal.org/  
Packages used: Kernel and 3D Triangulations  
License: package-dependent GNU LGPL v3+ / GNU GPL v3+; 3D Triangulations is GPL.

The application is therefore released under GPL-3.0-or-later. Refer to the
license files in the exact CGAL distribution used for a build.

## Qt

Project: https://www.qt.io/  
License: GNU LGPL v3 / GNU GPL v3 or a commercial Qt license, depending on the
Qt distribution used.

The source repository does not redistribute Qt binaries. A binary release must
include the applicable Qt notices and satisfy the requirements of the selected
Qt license.

## Boost

Project: https://www.boost.org/  
License: Boost Software License 1.0.

## Eigen

Project: https://eigen.tuxfamily.org/  
License: Mozilla Public License 2.0, with the Eigen exception where applicable.

## Ceres Solver

Project: https://ceres-solver.org/  
License: BSD 3-Clause.

## Google glog and gflags

Projects: https://github.com/google/glog and https://github.com/gflags/gflags  
License: BSD 3-Clause.

## oneTBB

Project: https://github.com/uxlfoundation/oneTBB  
License: Apache License 2.0.

## METIS

Project: https://github.com/KarypisLab/METIS  
License: Apache License 2.0 for current upstream releases; verify the license of
the exact version distributed by the package manager.

## nanoflann

The vendored `FeedBackSfMPrj/nanoflann.h` retains its BSD license and copyright
notice in the source file.

## progressbar

The vendored `FeedBackSfMPrj/progressbar.h` retains its MIT license and
copyright notice in the source file.

## Icons and UI media

Attributions and licenses for vendored UI media are retained in
`FeedBackSfMPrj/media/LICENSE`.

