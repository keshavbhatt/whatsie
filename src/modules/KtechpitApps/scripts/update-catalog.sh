#!/usr/bin/env bash
# Refreshes the bundled copy of the catalog (resources/catalog.json), its icons and the .qrc
# from the live endpoint. Run it before a release of the module; apps fall back to this copy
# until their first download of the catalog succeeds.
#
#   scripts/update-catalog.sh [catalog-url]
set -euo pipefail

DIR="$(cd "$(dirname "$0")/.." && pwd)"
URL="${1:-https://ktechpit.com/USS/public/api/apps.php}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

curl -fsSL --max-time 60 "$URL" -o "$TMP/catalog.json"
python3 -I - "$TMP/catalog.json" <<'PY'
import json, sys
d = json.load(open(sys.argv[1]))
assert d.get("schema") == 1, "unexpected schema"
assert len(d.get("apps", [])) >= 5, "too few apps"
PY
cp "$TMP/catalog.json" "$DIR/resources/catalog.json"

rm -f "$DIR"/resources/icons/*.png
python3 -I -c '
import json, sys
for a in json.load(open(sys.argv[1]))["apps"]:
    if a.get("icon"):
        print(a["id"], a["icon"])
' "$DIR/resources/catalog.json" | while read -r id url; do
    curl -fsSL --max-time 30 "$url" -o "$TMP/$id" || { echo "no icon for $id" >&2; continue; }
    magick "$TMP/$id" -resize 64x64 -background none -gravity center -extent 64x64 \
        "PNG32:$DIR/resources/icons/$id.png"
done

{
    echo '<RCC>'
    echo '  <qresource prefix="/ktechpit-apps">'
    echo '    <file alias="catalog.json">resources/catalog.json</file>'
    for f in "$DIR"/resources/icons/*.png; do
        n="$(basename "$f")"
        echo "    <file alias=\"icons/$n\">resources/icons/$n</file>"
    done
    echo '  </qresource>'
    echo '</RCC>'
} > "$DIR/ktechpit_apps.qrc"

echo "catalog: $(python3 -I -c 'import json,sys;print(len(json.load(open(sys.argv[1]))["apps"]))' "$DIR/resources/catalog.json") apps"
