#!/usr/bin/env bash
# Builds whatsie on the host against the KDE Qt 6.11 snap SDK
# (kde-qt6-core24-sdk) — the same Qt the shipped snap consumes at runtime
# from the kf6-core24 content snap and that Flathub's KDE runtime provides.
# It uses the cmake snap, as the snap build does, never the host's cmake.
# Dev/prod parity: what compiles here is what ships.
#
#   scripts/dev-build.sh            # RelWithDebInfo into ./build
#   WHATSIE_BUILD_TYPE=Debug scripts/dev-build.sh
#   WHATSIE_CMAKE_ARGS="-DWHATSIE_WERROR=ON" scripts/dev-build.sh
#   scripts/dev-build.sh --tests    # also run ctest afterwards
set -euo pipefail

DIR="$(cd "$(dirname "$0")/.." && pwd)"
SDK=/snap/kde-qt6-core24-sdk/current
SDK_LIB="$SDK/usr/lib/x86_64-linux-gnu"
BUILD="${WHATSIE_BUILD_DIR:-$DIR/build}"
CMAKE=/snap/cmake/current/bin/cmake

# shellcheck source=snap-runtime-env.sh
source "$DIR/scripts/snap-runtime-env.sh"

if [ ! -d "$SDK_LIB/cmake/Qt6" ]; then
    echo "Qt SDK snap missing. Install with:" >&2
    echo "  sudo snap install kde-qt6-core24-sdk" >&2
    exit 1
fi
if [ ! -x "$CMAKE" ]; then
    echo "cmake snap missing (the cmake the snap build uses). Install with:" >&2
    echo "  sudo snap install cmake --classic" >&2
    exit 1
fi
if [ ! -d "$WHATSIE_RT/usr/lib/x86_64-linux-gnu" ]; then
    echo "kf6-core24 runtime snap missing (needed to run/test). Install with:" >&2
    echo "  sudo snap install kf6-core24" >&2
    exit 1
fi

whatsie_prepare_runtime_farm "$BUILD"

# The SDK's own build tools (moc, rcc, qmlimportscanner, ...) need the SDK's
# libraries, some of which sit in the libproxy subdirectory. A few libraries
# those need are not on every host: link just the missing ones from core24 or
# the runtime snap.
sdk_path="$SDK_LIB:$SDK_LIB/libproxy"
build_libs="$BUILD/build-libs"
mkdir -p "$build_libs"
missing="$(for tool in "$SDK"/usr/bin/qt6/* "$SDK"/usr/lib/qt6/libexec/*; do
    [ -x "$tool" ] && LD_LIBRARY_PATH="$sdk_path" ldd "$tool" 2>/dev/null
done | awk '/=> not found/ {print $1}' | sort -u)"
for lib in $missing; do
    for from in "$WHATSIE_CORE24_BASE" "$WHATSIE_RT/usr/lib/x86_64-linux-gnu"; do
        if [ -e "$from/$lib" ]; then
            ln -sf "$from/$lib" "$build_libs/$lib"
            break
        fi
    done
done
export LD_LIBRARY_PATH="$sdk_path:$build_libs${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
link_path="$SDK_LIB:$WHATSIE_RT/usr/lib/x86_64-linux-gnu:$WHATSIE_CORE24_BASE"

# Extra CMake arguments: WHATSIE_CMAKE_ARGS="-DWHATSIE_WERROR=ON -DFOO=bar"
read -r -a EXTRA_ARGS <<< "${WHATSIE_CMAKE_ARGS:-}"

# The SDK keeps its CMake packages under lib/x86_64-linux-gnu, which CMake
# searches only on a Debian-style host; elsewhere Qt's packages (and its moc,
# rcc, uic) would come from the host's Qt, mixing two Qt versions in one link.
# "$SDK_LIB/cmake" finds them in the SDK, and -U drops any found on the host
# before.
"$CMAKE" -S "$DIR" -B "$BUILD" -G Ninja \
    -U 'Qt6*_DIR' \
    -DCMAKE_BUILD_TYPE="${WHATSIE_BUILD_TYPE:-RelWithDebInfo}" \
    -DCMAKE_PREFIX_PATH="$SDK_LIB/cmake;$SDK/usr" \
    -DQt6_DIR="$SDK_LIB/cmake/Qt6" \
    -DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath-link,$link_path -Wl,--allow-shlib-undefined" \
    -DCMAKE_SHARED_LINKER_FLAGS="-Wl,-rpath-link,$link_path" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DWHATSIE_SNAP_RUNTIME="$WHATSIE_RT" \
    -DWHATSIE_SNAP_RUNTIME_LIB_PATH="$(whatsie_runtime_lib_path "$BUILD")" \
    "${EXTRA_ARGS[@]}"
"$CMAKE" --build "$BUILD" -j"$(nproc)"

# clangd / IDEs look for compile_commands.json at the repo root.
ln -sf "$BUILD/compile_commands.json" "$DIR/compile_commands.json"

if [ "${1:-}" = "--tests" ]; then
    "$DIR/scripts/dev-run.sh" --ctest
fi
