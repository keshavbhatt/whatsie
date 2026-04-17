# Quick Reference: Building Whatsie with CMake

## TL;DR - Getting Started in 30 seconds

```bash
# Clone and navigate
git clone https://github.com/keshavbhatt/whatsie.git
cd whatsie

# Build (Release mode)
make build-release

# Run
./build/whatsie

# Install to system (optional)
make install
```

## Common Commands

### Building

```bash
# Debug build
make build-debug

# Release build (optimized)
make build-release

# Just configure (don't build)
CMAKE_BUILD_TYPE=Release make configure

# Build with specific number of threads
JOBS=8 make build-release

# Build in custom directory
BUILD_DIR=mybuild make build-release
```

### Running

```bash
# Run from build directory
make run

# Run directly
./build/whatsie

# Run from system (after install)
whatsie
```

### Installation & Uninstallation

```bash
# Install to /usr/local (default, doesn't need sudo)
make install

# Install to /usr (needs sudo for most systems)
INSTALL_PREFIX=/usr make install

# Or manually:
cd build && sudo cmake --install . --prefix /usr

# To uninstall (manual cleanup needed as CMake doesn't track uninstall):
sudo rm /usr/bin/whatsie
sudo rm -r /usr/share/org.keshavnrj.ubuntu/WhatSie/
# ... etc
```

### Cleaning

```bash
# Remove all build artifacts
make clean

# Or manually
rm -rf build/
```

## Direct CMake Commands (Without Makefile)

```bash
# Create build directory
mkdir build && cd build

# Configure for Release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..

# Configure for Debug
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build with Ninja
ninja

# Build with Make
cmake --build . --parallel 4

# Install
ninja install
# or
cmake --install . --config Release --prefix /usr/local
```

## Environment Variables

```bash
# Set default build type
export CMAKE_BUILD_TYPE=Release

# Set default install prefix
export INSTALL_PREFIX=/usr

# Set default number of build jobs
export JOBS=8

# Set Qt path (if not in standard location)
export CMAKE_PREFIX_PATH=/path/to/qt6/lib/cmake
```

## IDE Usage

### Qt Creator
1. File → Open File or Project
2. Select `CMakeLists.txt`
3. Qt Creator auto-configures
4. Select kit and build type in bottom-left
5. Ctrl+B to build

### VS Code
1. Install "CMake" extension
2. Install "CMake Tools" extension
3. Open folder containing `CMakeLists.txt`
4. Select a kit when prompted
5. F7 to build

### CLion
1. Open project folder
2. CLion auto-detects CMakeLists.txt
3. Configure build configuration in preferences
4. Ctrl+Shift+F10 to build and run

## Build Options

```bash
# Flatpak build (no dictionary compilation)
cmake -DFLATPAK_BUILD=ON ..

# Use Unix Makefiles instead of Ninja
cmake -G "Unix Makefiles" ..

# Set custom installation prefix
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..

# All together
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DFLATPAK_BUILD=OFF \
      -G Ninja ..
```

## Troubleshooting Quick Tips

| Problem | Solution |
|---------|----------|
| Qt6 not found | `export CMAKE_PREFIX_PATH=/usr/lib/cmake/Qt6` |
| Ninja not found | `sudo apt install ninja-build` |
| Permission denied on install | Add `sudo` or use prefix in home dir |
| qwebengine_convert_dict missing | Ignore warning or use `-DFLATPAK_BUILD=ON` |
| Build fails with C++ errors | Update compiler: `sudo apt install build-essential` |

## Build Output Locations

```
build/
├── whatsie                      # Main executable
├── CMakeFiles/                  # CMake temporary files
├── cmake_install.cmake          # Installation script
├── compile_commands.json        # For IDE language servers
└── qtwebengine_dictionaries/    # Compiled spell-check dicts (if built)
```

## Performance Tips

- Use **Ninja** instead of Make (faster)
- Use **Release** build for production (`CMAKE_BUILD_TYPE=Release`)
- Use **-j flag** to parallelize: `JOBS=16 make build-release`
- Use **ccache** for faster rebuilds: `cmake -DCMAKE_C_COMPILER_LAUNCHER=ccache ..`

## Version & Build Info

Check version and build information:
```bash
./build/whatsie --version
./build/whatsie --build-info
```

## Development Workflow

1. **Initial setup**:
   ```bash
   make build-debug
   ```

2. **Iterative development**:
   ```bash
   # Make source code changes
   make build          # Rebuild incrementally
   make run            # Test changes
   ```

3. **Before committing**:
   ```bash
   make clean
   make build-release
   ./build/whatsie     # Final test
   ```

4. **Installation testing**:
   ```bash
   make install
   whatsie             # Test from any directory
   ```

## Further Help

- `make help` - Show all available targets
- `CMAKE_MIGRATION.md` - Detailed migration guide
- `MIGRATION_SUMMARY.md` - Complete feature summary
- `CMakeLists.txt` - Source configuration (well-commented)

