#!/bin/bash
POSSIBLE_PATHS=(
    "/usr/lib/qt6/libexec/qwebengine_convert_dict"
    "/usr/lib/qt6/qwebengine_convert_dict"
    "/usr/lib/qt6/bin/qwebengine_convert_dict"
)

for path in "${POSSIBLE_PATHS[@]}"; do
    if [[ -x "$path" ]]; then
        exec "$path" "$@"
        exit 0
    fi
done

echo "qwebengine_convert_dict not found in any specified paths" >&2
exit 1
