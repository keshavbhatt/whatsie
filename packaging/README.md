# Packaging

Filled in during milestone M4 (see `DOCS/ROADMAP.md`). Decisions already taken:

| Target | Runtime | Notes |
|---|---|---|
| Snap | core24 + `kde-neon-6` extension → `kf6-core24` content snap (Qt 6.11) | `browser-support` with `allow-sandbox: true` (ADR-008). Plugs audited per release; IME plugins bundled (FEATURES L4). Build recipe mirrors `scripts/dev-build.sh`. |
| Flatpak | `org.kde.Platform` 6.x (Qt 6.11) | Portals for file chooser and notifications. No `--filesystem=home`. |
| AppImage / deb / rpm / AUR / Windows / macOS | — | Only with a committed maintainer; see FEATURES M11 for codec caveats. |

Application id: `org.keshavbhatt.whatsie` (pending OQ-1 in `DOCS/DECISIONS.md`).
