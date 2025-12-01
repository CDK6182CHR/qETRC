# Build qETRC on ArchLinux

This guide provides instructions for building qETRC on ArchLinux systems.

## Requirements

Install the required packages using pacman:

```bash
sudo pacman -S base-devel cmake qt6-base qt6-tools qt6-declarative qt6-svg
```

Additionally, you may need:
```bash
sudo pacman -S qt6-xcb-private-headers  # For platform headers
```

## Get the source codes

```bash
git clone https://github.com/CDK6182CHR/qETRC
cd qETRC
git submodule update --init --recursive
```

## Build dependencies

### Build and Install SARibbon

Since SARibbon is not available in ArchLinux repositories, you need to build it from the submodule:

```bash
cd external/SARibbon
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)
sudo make install
```

## Build qETRC

Once SARibbon is installed, return to the main project directory and build qETRC:

```bash
cd /path/to/qETRC  # Replace with your actual qETRC directory
mkdir build && cd build
cmake .. -DCMAKE_CXX_FLAGS="-I/usr/include/qt6/QtGui/6.10.0/QtGui" -DSARibbonBar_DIR=/usr/lib/cmake/SARibbonBar
make -j$(nproc)
```

## Troubleshooting

### Missing qplatformnativeinterface.h

If you encounter errors about missing `qpa/qplatformnativeinterface.h`, it's due to the specific include directory structure in ArchLinux Qt6 packages. The solution is to add the version-specific include path as shown in the cmake command above.

### Advanced Docking System (ads) build issues

If you encounter issues with ads library that requires Qt platform headers, ensure you have installed `qt6-xcb-private-headers` package.

## Final Notes

- The built executable will be in `build/qETRC`
- This has been tested on ArchLinux with Qt6 6.10.0
- The CMAKE_CXX_FLAGS approach is a workaround for the versioned include structure in ArchLinux Qt6 packages