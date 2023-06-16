# Build qETRC with CMake

Since June 16, 2023, the building system of `qETRC` project switch to `CMake`. The old building system based on `qmake` is not supported any longer.

To build this project with `CMake`, follow the instructions below.

## Requirements

- Qt5. The current developing environment is Qt 5.15.2, but older Qt5 versions or Qt6 are in principle supported (not tested yet).
- Compiler that supports C++20.
- CMake >= 3.14.

The developing environment and building system:

- Microsoft Visual Studio 2019 for win64 version
- Qt Creator 9.0.1 for Android version

## External libraries

Currently, the qETRC project relies on two external (third-party) libraries: [SARibbon](https://github.com/czyt1988/SARibbon) and [Qt-Advanced-Docking-System](https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System) (ads). For building hosts that have network connections, it is easy to build with automatic procedures. 

## Get the source codes

Assuming you are working on a building host machine with `git` installed. Create a directory for build, and clone the project 

```shell
git clone https://github.com/CDK6182CHR/qETRC
```

or

```
git clone https://gitee.com/xep0268/qETRC
```

Then synchronize the submodule to get `SARibbon` code

```shell
git submodule sync
```

Then just follow the ordinary `CMake` building procedure with IDE. You may need to provide your installation path to Qt.