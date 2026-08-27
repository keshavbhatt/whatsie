#!/usr/bin/env bash
# Runs the dev build against the kf6-core24 runtime content snap — the very
# libraries the shipped whatsie snap uses. Host fonts, display and D-Bus work
# directly; Chromium's sandbox stays ON.
#
#   scripts/dev-run.sh                 # launch the app
#   scripts/dev-run.sh --ctest [args]  # run the test suite (env baked in by CMake)
#   scripts/dev-run.sh -- --some-flag  # pass arguments to whatsie
set -euo pipefail

DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${WHATSIE_BUILD_DIR:-$DIR/build}"

# shellcheck source=snap-runtime-env.sh
source "$DIR/scripts/snap-runtime-env.sh"

[ -d "$WHATSIE_RT/usr/lib/x86_64-linux-gnu" ] || {
    echo "kf6-core24 runtime snap missing: sudo snap install kf6-core24" >&2; exit 1; }

if [ "${1:-}" = "--ctest" ]; then
    # ctest is a host binary: keep the host environment. The tests themselves
    # get the runtime env through their CTest ENVIRONMENT property
    # (tests/CMakeLists.txt, from WHATSIE_SNAP_RUNTIME*).
    cd "$BUILD"
    exec ctest --output-on-failure "${@:2}"
fi

whatsie_prepare_runtime_farm "$BUILD"
whatsie_export_runtime_env "$BUILD"
export QT_LOGGING_RULES="${QT_LOGGING_RULES:-whatsie.*.debug=true}"

[ "${1:-}" = "--" ] && shift
cd "$DIR"
exec "$BUILD/src/whatsie" "$@"
