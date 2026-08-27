# Packaging

Filled in during milestone M4 (see `DOCS/ROADMAP.md`). Decisions already taken:

| Target | Runtime | Notes |
|---|---|---|
| Snap | core24 + `kde-neon-6` extension → `kf6-core24` content snap (Qt 6.11) | `browser-support` with `allow-sandbox: true` (ADR-008). Plugs audited per release; IME plugins bundled (FEATURES L4). Build recipe mirrors `scripts/dev-build.sh`. |
| Flatpak | `org.kde.Platform` 6.x (Qt 6.11) | Portals for file chooser and notifications. No `--filesystem=home`. |
| Windows | Qt 6.11 msvc2022_64, `windeployqt` | MSI + SignPath signing (W#325); MSVC runtime bundled (Y#68). ADR-016. |
| AppImage / deb / rpm / AUR / macOS | — | Not release targets; only with a committed maintainer. |

Application id: `com.ktechpit.whatsie` (ADR-014).
