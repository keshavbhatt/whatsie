# CMake Migration Guide for Whatsie

## Overview
This document outlines the migration from qmake to CMake for building Whatsie with Qt6.

## Prerequisites
- CMake 3.24 or later
- Qt6 (6.0 or later) with development packages
- Ninja build tool (optional but recommended)
- C++17 compatible compiler (GCC 7+, Clang 5+, or MSVC 2017+)

### Install build dependencies on Ubuntu/Debian:
```bash
sudo apt-get install cmake ninja-build qt6-base-dev qt6-webengine-dev qt6-positioning-dev libx11-dev
```

### Install build dependencies on Fedora:
```bash
sudo dnf install cmake ninja-build qt6-qtbase-devel qt6-qtwebengine-devel libxkbcommon-devel libX11-devel
```

### Install build dependencies on Arch:
```bash
sudo pacman -S cmake ninja qt6-base qt6-webengine
```

## Building with CMake

### Basic Build (Debug)
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
ninja
```

### Release Build
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
ninja
```

### Build with specific install prefix
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
ninja
```

### Build for Flatpak (skip dictionary compilation)
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DFLATPAK_BUILD=ON ..
ninja
```

### Using Make instead of Ninja
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" ..
make -j4
```

## Installation

### Install to system (requires sudo)
```bash
cd build
sudo ninja install
# or
sudo cmake --install . --config Release --prefix /usr
```

### Install to local prefix
```bash
cd build
ninja install
# or
cmake --install . --config Release --prefix ~/.local
```

## Running the Application

### From build directory
```bash
./build/whatsie
```

### After installation to /usr
```bash
whatsie
```

## Supported CMake Options

- `CMAKE_BUILD_TYPE`: Debug or Release (default: Release)
- `CMAKE_INSTALL_PREFIX`: Installation prefix (default: /usr/local)
- `FLATPAK_BUILD`: Skip dictionary handling (ON/OFF, default: OFF)
- `CMAKE_GENERATOR`: Build system generator (Ninja or Unix Makefiles)

## Building in Parallel

CMake automatically detects available CPU cores. To explicitly set parallel jobs:

```bash
cd build
ninja -j 8              # Use 8 jobs with Ninja
# or
make -j 8               # Use 8 jobs with Make
# or
cmake --build . -j 8    # Platform-agnostic
```

## Troubleshooting

### Qt6 not found
Ensure Qt6 is installed and in your PATH. You may need to set:
```bash
export CMAKE_PREFIX_PATH=/path/to/qt6/lib/cmake
```

### qwebengine_convert_dict not found
The script will warn if the dictionary compiler is not found. You can:
1. Install it manually: `sudo apt-get install qtwebengine5-dev-tools` (for Qt5, adapt for Qt6)
2. Build with `-DFLATPAK_BUILD=ON` to skip dictionary compilation

### Permission denied when running whatsie
After installation to /usr, the binary should be executable. If not:
```bash
sudo chmod +x /usr/bin/whatsie
```

## Cleaning Build Artifacts

```bash
rm -rf build
```

## IDE Integration

### Qt Creator
Qt Creator automatically recognizes CMake projects. Open the CMakeLists.txt file and it should configure automatically.

### VS Code with CMake Tools extension
1. Install the CMake extension
2. Open the folder
3. Select a kit
4. Build with Ctrl+Shift+B

### CLion
Open the project root directory and CLion will automatically detect the CMakeLists.txt.

## Migration Notes

The migration includes:
- Automated MOC (Meta-Object Compiler) processing
- Automated UIC (User Interface Compiler) processing
- Automated resource compilation (RCC)
- Proper inclusion of all source files and libraries
- Git information extraction at build time
- Dictionary compilation support (when available)
- Full installation target configuration

## Next Steps

After successful CMake migration:
1. Verify the application runs correctly
2. Test all features
3. Check installation paths and permissions
4. Update CI/CD pipelines if applicable
5. Consider Qt6-specific deprecation warnings in code

## Additional Resources

- CMake Documentation: https://cmake.org/cmake/help/latest/
- Qt6 Documentation: https://doc.qt.io/qt-6/
- CMake Qt Documentation: https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html

