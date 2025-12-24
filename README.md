![build_cmake_status](https://github.com/niksn418/julia-fractal-gl-qt/actions/workflows/build_cmake.yml/badge.svg)

# Julia set OpenGL renderer

App to draw Julia Set fractal using Qt5 and OpenGl 3.3. Supports:
- zooming and moving (via mouse scroll and mouse dragging respectively);
- configuring fractal, e.g. changing its `c` parameter;;
- configuring rendering quality via specifying number of iterations and escape radius.

## Requirements

- git [https://git-scm.com](https://git-scm.com);
- C++20 compatible compiler;
- CMake 3.10+ [https://cmake.org/](https://cmake.org/);
- Qt 5 [https://www.qt.io/](https://www.qt.io/);
- (Optionally) Your favourite IDE;
- (Optionally) Ninja build [https://ninja-build.org/](https://ninja-build.org/).

## Hardware requirements

- GPU with OpenGL 3+ support.

## Build from console

- Clone this repository `git clone <url> <path>`;
- Go to root folder `cd <path-to-repo-root>`;

Now, if on POSIX OS and Qt is in PATH, you can run `./build_and_run.sh`. Or, manually:
- Run CMake `cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release`;
    - If you want, you can specify generator name via `-G <generator-name>`;
    - If Qt is not found by CMake, try to specify path to it via `-DCMAKE_PREFIX_PATH=<path-to-qt-installation>`;
- Run build `cmake --build cmake-build-release -j<number-of-threads-to-build>`;
    - To build and run, add `-t run`;
    - The other way, f.e. via Ninja generator, looks like `cd cmake-build-release; ninja -j<number-of-threads-to-build>`.

## Build with MSVC

- Clone this repository `git clone <url> <path>`;
- Open root folder in IDE;
- Build, possibly specify build configurations and path to Qt library.

## Run and debug

- Since we link with Qt dynamically don't forget to add `<qt-path>/<abi-arch>/bin` and `<qt-path>/<abi-arch>/plugins/platforms` to `PATH` variable.
