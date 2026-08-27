# Roadmap

Milestones are ordered by dependency, not by calendar. Each milestone lists its feature rows
(from `FEATURES.md`) and an exit criterion. A milestone is done when every listed row is `done`
in `FEATURES.md`, the tests pass under `scripts/dev-run.sh --ctest`, and `PROGRESS.md` has the
entry.

Rows marked **NICE** are included only after the owner marks them `KEEP` in `FEATURES.md`.

---

## M0 — Foundation ✅ (2026-08-27)

- Layered CMake scaffold (`core` → `web`/`platform` → `ui` → `app`), Qt 6.11 via snap SDK,
  C++20, strict warnings, clang-format/clang-tidy configs.
- `core::Settings` facade with tests; `web::WebProfile`/`WebPage`/`WebView`; thin `MainWindow`;
  page console → logging; UA sanitiser with tests; offscreen smoke test.
- DOCS set: coding standards, lessons, ADRs, feature diff, this roadmap, progress log,
  reference analyses + GitHub issue digest.
- **Exit:** `scripts/dev-build.sh --tests` green; app shows WhatsApp Web login on Wayland. ✅

## M1 — Usable shell (Linux) ✅ (2026-08-27)

Goal: replaces whatsie for daily use *without* notifications yet.
Delivered: see `PROGRESS.md` 2026-08-27 M1 entry. Deferred into M3 as planned: page-side theme
sync, bridge-based unread count, in-app call pop-out.

| Rows | Work |
|---|---|
| S8, S9, X1 | `app::SingleInstance` (QLocalServer + JSON), `QCommandLineParser` with `--profile`, `--minimized`, `--log-file`, `--new-chat`, `--settings`, `--quit`; profile-aware paths. |
| P10 | Logging to file (`--log-file` + default under `AppDataLocation/logs`), ring buffer, `platform::describeHost()` in About → "Copy diagnostics". |
| S2, S3, S4, S5, S6, S7 | `ui::WindowStateController`: geometry+screen, close-to-tray vs quit, start minimized, no-tray guard, `commitDataRequest`, fullscreen enter/exit incl. page-requested. |
| T1, T2, T4 | `ui::TrayController` + `core::UnreadBadge` (pure image composition, tested); unread count from title now, bridge later (M3). |
| A1, A2, A6 | `core::ThemeService` (System/Light/Dark, follows `QStyleHints`), Qt palette application, page theme via profile script (M3 hook), zoom controller with maximized profile and min-size scaling. |
| M4, M13, S12 | Navigation policy (external links), context-menu filter, render-crash back-off. |
| S10 | `whatsapp://` handling → `/send?phone=` URL built in C++. |
| D1 | About dialog (code-built). |
| — | Settings dialog skeleton: 4 tabs, options only for rows above. |

**Exit:** login, chat, restore from tray, theme switch, zoom, reopen with saved geometry,
`whatsie --new-chat 123` raises the running instance. Tests for every `core/` class.

## M2 — Notifications

| Rows | Work |
|---|---|
| N1, D5 | `core::INotifier`; `platform::linux::FreedesktopNotifier` (QDBus, id tracking, `image-data` RGBA8888, `desktop-entry`, `category`, actions); identicon with stable hash; unit tests with a fake D-Bus interface. |
| N6, N11, N10 | Presenter wiring on the profile; auto-grant Notification permission; click → raise (chat focus delegated to WA's own `onclick`). |
| N5, N7 (if KEEP) | sound hint; manual DND in tray. |
| N2 | `platform::windows::TrayNotifier` (can land in M6 if Windows CI is not ready). |

**Exit:** GNOME + KDE show native notifications with avatar; clicking raises the window;
unrelated notification clicks do nothing (regression test on id matching).

## M3 — Web integration layer

| Rows | Work |
|---|---|
| ADR-006 | `web::Bridge` (QWebChannel), `web::ScriptBundle` loader (profile-level, DocumentCreation/Ready), `scripts/bridge.js`, syntax test over all scripts. |
| S1 (storage) | `storage-persist.js` (navigator.storage.persist lie). |
| T2 | `unread-count.js` reporting through the bridge (title fallback stays). |
| A1 | `theme-preload.js` sets WA `localStorage.theme` at DocumentCreation from config. |
| A6 | `ctrl-wheel-block.js`. |
| M1, M2, M3 | Permission controller (signals out, UI answers), desktop-media picker dialog, in-app call pop-out window with Esc/close. |
| M5, M7, M8, M9 | File chooser (portal), clipboard image paste rescue, minimal download flow + notification, mute. |
| S13/S14/S16 (if KEEP) | Connection watchdog policy (pure, tested) + reload; network-resume reload; SW recovery. |
| P5, P6 | Storage manager with `isSafeToDelete` guard; hardware-acceleration setting (single restart-required option). |
| A8, A14, M6 (if KEEP) | privacy blur script; smooth scrolling default; drag-drop pipeline. |

**Exit:** voice/video call with camera+mic works; screen share picker appears; file attach via
dialog, paste and (if KEEP) drag-drop; theme survives restart 20/20 times (scripted test).

## M4 — Packaging & CI

| Target | Work |
|---|---|
| Snap | `packaging/snap/snapcraft.yaml`: core24, `kde-neon-6` extension (Qt 6.11 from `kf6-core24`), `browser-support` with `allow-sandbox: true`, plugs audited (`home`, `camera`, `audio-*`, `removable-media`, …), IME plugins (L4). Edge channel from CI. |
| Flatpak | `packaging/flatpak/org.keshavbhatt.whatsie.yml` on `org.kde.Platform` 6.x (Qt 6.11), portals for files/notifications (N3). |
| AppImage | Optional; only if a maintainer commits to it (codec caveats, M11). |
| CI | GitHub Actions: build with `WHATSIE_WERROR=ON` against the SDK snap, run ctest offscreen, clang-format check, snap build → edge, flatpak build. |
| Release | Version from `project(VERSION)`; AppStream `<releases>` generated from `CHANGELOG.md`. |

**Exit:** `snap install whatsie --edge` works on a clean Ubuntu 24.04 VM; Flatpak builds locally.

## M5 — Approved extras (decided 2026-08-27)

Each is its own small PR with tests and a `FEATURES.md` status update. Order by value/cost:

1. **Spell check** L1 (system-language `.bdic` at build) → L2 (on-demand download list). ADR-017.
2. **Autostart** P4 (XDG desktop entry; Windows registry in M6).
3. **Proxy** P3 + proxy-auth dialog M12b.
4. **Privacy blur** A8; **interface scale** A7; **smooth scrolling option** A14 (default off).
5. **Tray options** T3 (symbolic icon), T5 (hide tray), T6 (connection-dim, needs S13).
6. **DND** N7 (manual), **sound hint** N5, **portal notifications** N3.
7. **Shortcuts sheet** S26; **wa.me / invite links** S11.
8. **App lock** P1, hardened per ADR-015 — last, because it touches every window and the IPC layer.

Planned after v1 (LATER): S23 global hotkey (portal), T8 launcher badge, P8 unload-when-hidden,
X2 account tabs.

## M6 — Windows (ADR-016)

`platform/windows/` backends: notifications (N2, tray toast), autostart (registry), tray
click focus grace (T4), taskbar badge (T8, later). Packaging: MSI with bundled MSVC runtime
(Y#68), SignPath signing (W#325), `windeployqt`; CI job on `windows-latest` with Qt 6.11
msvc2022_64. No `#ifdef` inside business logic (ADR-003). macOS is out of scope.

## Release criteria for 5.0.0

1. All CORE rows `done`; no `DROP` row has code in the tree.
2. `ctest` green in CI; `core/` ≥ 80 % line coverage.
3. Zero known data-destroying paths (ADR-011 audit).
4. Snap + Flatpak published; README build section matches CI.
5. `DOCS/FEATURES.md` statuses accurate; `CHANGELOG.md` written.
