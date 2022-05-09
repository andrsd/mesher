# MESHER

Simple Qt-based graphical front end for meshing 2D and 3D domains for finite element analysis.

![main-wnd](https://github.com/andrsd/mesher/raw/main/docs/imgs/mesher.png)

Features:
- Generate triangular meshes using [Triangle](https://www.cs.cmu.edu/~quake/triangle.html)
- Generate tetrahedral meshes using [TetGen](https://wias-berlin.de/software/tetgen/)
- Support for `poly` files (Triangle and TetGen)
- Support for `STL` files
- Export to `ExodusII` file format
- Multiplatform:
  - MacOS X,
  - linux (not tested), and
  - Windows (not tested)

Using:
- VTK9
- Qt5
- [meshpy](https://github.com/inducer/meshpy)

[![qa](https://github.com/andrsd/mesher/actions/workflows/flake8.yml/badge.svg?branch=main)](https://github.com/andrsd/mesher/actions/workflows/flake8.yml)
[![test](https://github.com/andrsd/mesher/actions/workflows/build.yml/badge.svg)](https://github.com/andrsd/mesher/actions/workflows/build.yml)
[![codecov](https://codecov.io/gh/andrsd/mesher/branch/main/graph/badge.svg?token=RHCTM3I44O)](https://codecov.io/gh/andrsd/mesher)
[![License](http://img.shields.io/:license-mit-blue.svg)](https://andrsd.mit-license.org/)
[![Scc Count Badge](https://sloc.xyz/github/andrsd/mesher/)](https://github.com/andrsd/mesher/)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/g/andrsd/mesher.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/andrsd/mesher/context:python)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/1cdafdee05c548c382ddce93605357fe)](https://www.codacy.com/gh/andrsd/mesher/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=andrsd/mesher&amp;utm_campaign=Badge_Grade)
