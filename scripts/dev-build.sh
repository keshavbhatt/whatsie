#!/usr/bin/env bash
# Builds whatsie on the host against the KDE Qt 6.11 snap SDK
# (kde-qt6-core24-sdk) — the same Qt the shipped snap consumes at runtime
# from the kf6-core24 content snap and that Flathub's KDE runtime provides.
# Dev/prod parity: what compiles here is what ships.
#
#   scripts/dev-build.sh            # RelWithDebInfo into ./build
#   WHATSIE_BUILD_TYPE=Debug scripts/dev-build.sh
#   WHATSIE_CMAKE_ARGS="-DWHATSIE_WERROR=ON" scripts/dev-build.sh
#   scripts/dev-build.sh --tests    # also run ctest afterwards
set -euo pipefail

DIR="$(cd "$(dirname "$0")/.." && pwd)"
SDK=/snap/kde-qt6-core24-sdk/current
BUILD="${WHATSIE_BUILD_DIR:-$DIR/build}"

# shellcheck source=snap-runtime-env.sh
source "$DIR/scripts/snap-runtime-env.sh"

if [ ! -d "$SDK/usr/lib/x86_64-linux-gnu/cmake/Qt6" ]; then
    echo "Qt SDK snap missing. Install with:" >&2
    echo "  sudo snap install kde-qt6-core24-sdk" >&2
    exit 1
fi
if [ ! -d "$WHATSIE_RT/usr/lib/x86_64-linux-gnu" ]; then
    echo "kf6-core24 runtime snap missing (needed to run/test). Install with:" >&2
    echo "  sudo snap install kf6-core24" >&2
    exit 1
fi

# The SDK's own build tools (moc, rcc, qmlimportscanner, ...) need the SDK's
# libraries, some of which sit in the libproxy subdirectory.
export LD_LIBRARY_PATH="$SDK/usr/lib/x86_64-linux-gnu:$SDK/usr/lib/x86_64-linux-gnu/libproxy${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# Extra CMake arguments: WHATSIE_CMAKE_ARGS="-DWHATSIE_WERROR=ON -DFOO=bar"
read -r -a EXTRA_ARGS <<< "${WHATSIE_CMAKE_ARGS:-}"

whatsie_prepare_runtime_farm "$BUILD"

cmake -S "$DIR" -B "$BUILD" \
    -DCMAKE_BUILD_TYPE="${WHATSIE_BUILD_TYPE:-RelWithDebInfo}" \
    -DCMAKE_PREFIX_PATH="$SDK/usr" \
    -DQt6_DIR="$SDK/usr/lib/x86_64-linux-gnu/cmake/Qt6" \
    -DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath-link,$SDK/usr/lib/x86_64-linux-gnu -Wl,--allow-shlib-undefined" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DWHATSIE_SNAP_RUNTIME="$WHATSIE_RT" \
    -DWHATSIE_SNAP_RUNTIME_LIB_PATH="$(whatsie_runtime_lib_path "$BUILD")" \
    "${EXTRA_ARGS[@]}"
cmake --build "$BUILD" -j"$(nproc)"

# clangd / IDEs look for compile_commands.json at the repo root.
ln -sf "$BUILD/compile_commands.json" "$DIR/compile_commands.json"

if [ "${1:-}" = "--tests" ]; then
    "$DIR/scripts/dev-run.sh" --ctest
fi
