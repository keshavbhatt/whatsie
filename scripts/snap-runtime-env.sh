#!/usr/bin/env bash
# Sourced by dev-build.sh / dev-run.sh. Provides:
#   WHATSIE_RT              runtime content snap root (kf6-core24)
#   WHATSIE_RT_LIB_PATH     LD_LIBRARY_PATH value for running dev binaries
#   whatsie_prepare_runtime_farm <build-dir>
#   whatsie_export_runtime_env  <build-dir>
#
# The dev build links against the kde-qt6-core24-sdk snap and runs against the
# kf6-core24 content snap — the exact libraries the shipped snap uses. Host
# glibc stays authoritative (the content snap carries no libc).

WHATSIE_RT=/snap/kf6-core24/current
WHATSIE_CORE24_BASE=/snap/core24/current/usr/lib/x86_64-linux-gnu

# Some runtime-snap libs depend on Ubuntu base libraries the host may not
# have (libselinux, liblerc, ...). Farm them from the core24 base snap,
# EXCLUDING the glibc family — the host's newer glibc must stay
# authoritative or the loader chokes on GLIBC_PRIVATE symbols.
whatsie_prepare_runtime_farm() {
    local build="$1"
    local farm="$build/core24-libs"
    mkdir -p "$farm"
    local f b
    for f in "$WHATSIE_CORE24_BASE"/*.so*; do
        b="$(basename "$f")"
        case "$b" in
            libc.so*|libm.so*|libmvec.so*|libpthread.so*|libdl.so*|librt.so*|\
            ld-linux*|libresolv.so*|libnsl.so*|libnss_*|libanl.so*|libutil.so*|\
            libBrokenLocale.so*|libthread_db.so*) continue ;;
        esac
        ln -sf "$f" "$farm/$b"
    done
}

whatsie_runtime_lib_path() {
    local build="$1"
    echo "$WHATSIE_RT/usr/lib/x86_64-linux-gnu:$WHATSIE_RT/usr/lib/x86_64-linux-gnu/libproxy:$build/core24-libs"
}

whatsie_export_runtime_env() {
    local build="$1"
    export LD_LIBRARY_PATH="$(whatsie_runtime_lib_path "$build")${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    export QT_PLUGIN_PATH="$WHATSIE_RT/usr/lib/x86_64-linux-gnu/qt6/plugins"
    export QTWEBENGINEPROCESS_PATH="$WHATSIE_RT/usr/lib/qt6/libexec/QtWebEngineProcess"
    export QTWEBENGINE_RESOURCES_PATH="$WHATSIE_RT/usr/share/qt6/resources"
    # This Qt build logs to journald when stderr is not a TTY; for dev runs we
    # want plain stderr (file-capturable).
    export QT_FORCE_STDERR_LOGGING=1
}
