# Build qETRC with CMake

Since June 16, 2023, the building system of `qETRC` project switch to `CMake`. The old building system based on `qmake` is not supported any longer.

Since Mar. 28, 2024, the dependency `SARibbonBar` library is updated to v2.0.3. Extra dependency `QWindowKits` is imported, the building system is updated.

To build this project with `CMake`, follow the instructions below.

## Requirements

- Qt6. The current developing environment is Qt 6.5.1. Qt 5 may be supported, but not tested.
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
git submodule update --init --recursive
```

## Build the dependencies

Currently, the `ads` dependency is built along with `qETRC` (provided network connections). Here, we need build `SARibbon` and its dependency manually.

See the doc in `SARibbon` repo: [czyt1988/SARibbon: Ribbon Control for Qt (github.com)](https://github.com/czyt1988/SARibbon).

Here, we provide a brief documentation about this.

### For windows

For Windows user, a script `build_SARibbon.bat` is provided at directory `external`. Run that script within the Qt developer prompt should build the `QWindowKit` and `SARibbon` dependencies. If such script fails, use the following workflow.

### For all platforms

#### `QWindowKit`

> This is only needed if `SARibbon` is built with `SARIBBON_USE_FRAMELESS_LIB=ON`, which is used by the win64 release and is recommended.

It is convenient to build `QWindowKit` (QWK) with the scripts provided by the author of `SARibbon`. 

Load the CMake project `external/SARibbon/src/SARibbonBar/3rdparty/CMakeLists.txt` with Visual Studio (VS) or QtCreator, then build and install the project. A folder named `bin_XXX` (e.g. `bin_qt6.5.1_MSVC_x64`) should appears at `external/SARibbon`.

#### `SARibbon`

Load the CMake project `external/SARibbon/CMakeLists.txt` with VS or QtCreator, then build and install the project. It is recommended to set `SARIBBON_USE_FRAMELESS_LIB=ON`, as addressed by the author of `SARibbon`. The library files should be installed to the `bin_XXX` directory as above.



## Build qETRC

Once the `SARibbon` lib is built, the `qETRC` project could be built by using normal workflows with CMake in VS or QtCreator. In CMake, you may need to provide the directory of `SARibbon` by set `SARibbonBar_DIR=path/to/qETRC/root/external/SARibbon/bin_XXX/lib/cmake/SARibbonBar`.