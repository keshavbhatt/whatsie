# Progress log

Newest first. One entry per work session. Keep entries factual: what changed, what was verified,
what is blocked. Milestone status table at the bottom.

---

## 2026-08-27 — M2 Notifications

**Done** (rows N1, N2, N3, N5, N6, N7, N10, N11, D5 marked `done`)
- `core/notifications/`: `Notification` model + `INotifier` interface; `NotificationService`
  (settings/DND gate, id assignment, identicon substitution, sound/timeout/desktop-entry
  defaults, primary→fallback retry on `failed`, activation only for known ids); `DndController`
  (1 h / 2 h / indefinite, timer-expired, not persisted); `identicon` with FNV-1a stable colour
  and initials. Settings: `notifications/enabled|sound|timeoutSec`.
- `platform/linux/`: `FreedesktopNotifier` over QtDBus (`Notify` with `desktop-entry`,
  `category=im.received`, `urgency`, `sound-name`, `image-data` as `(iiibiiay)` RGBA8888 —
  fixes the inverted-avatar bug W#312/328; `ActionInvoked`/`NotificationClosed` matched by
  server id — fixes the raise-on-any-click bug W#5/42/271/278); `PortalNotifier` for Flatpak
  with awaited reply so failure falls back (whatly's silent `NoBlock` bug); `notifier_factory`.
- `web/NotificationPresenter`: profile presenter → service; click → `QWebEngineNotification::click()`
  (WhatsApp opens the chat) + window raise. Notification permission auto-granted on request and
  pre-granted on the profile (W#307).
- `ui/`: `TrayNotifier` (balloon backend: Windows path and Linux last resort), `NotificationHub`
  (wires backend/service/presenter), tray "Do not disturb" submenu with tooltip state,
  Notifications settings tab (enable, sound, hide-after, "Send test notification").
- Tests: `tst_identicon`, `tst_notification_service` (fake backends: defaults, suppression, id
  filtering, fallback, DND expiry), `tst_freedesktop_notifier` (private `dbus-daemon`, fake
  server: hints, wire signature, RGBA bytes, id filtering, close). 14/14 pass.

**Verified**
- Real session (KDE Wayland): log shows `notification backend: "freedesktop" (fallback: tray)`.
- Test gotcha: host `dbus-daemon` must be spawned without the runtime-snap `LD_LIBRARY_PATH`.

**Known gaps**
- Portal backend untested outside Flatpak (M4 will exercise it in the Flatpak build).
- No per-chat "open this chat" beyond WhatsApp's own `onclick` (by design, N10).

---

## 2026-08-27 — M1 Usable shell

**Done** (all rows marked `done` in `FEATURES.md`)
- `core/`: `Settings` grew closeAction / startMinimized / trayLeftClickToggles /
  zoomFactorMaximized / smoothScrolling; pure modules `zoom_policy`, `unread_badge`,
  `navigation_policy` (whatsapp://, wa.me, api.whatsapp.com, plain numbers → `/send` URL),
  `render_crash_policy` (1 s/3 s/9 s then give up), `log_sink` (chained handler, 1000-line ring,
  rotating file), `theme/theme_service` (System/Light/Dark via `QStyleHints`). 7 test suites.
- `app/`: `cli_options` (`--profile -m --log-file --no-log-file -n --settings -q [url]`),
  `single_instance` (`QLocalServer` + newline-delimited JSON, key `whatsie-<profile>-<uid>`),
  `Application` parses CLI, applies profile suffix to the app name (own settings/data dirs),
  installs file logging, forwards commands when secondary, dispatches IPC as signals. 2 suites.
- `web/`: `WebPage` sends link clicks and `window.open` targets that leave web.whatsapp.com to the
  system browser (`PopupPage` helper), signals in-app pop-ups for M3; `WebView` owns zoom mode
  (normal vs maximized), Ctrl+wheel via child event filter, unread from title, crash back-off
  with reload, fullscreen requests, filtered context menu; profile follows smooth-scrolling live.
- `ui/`: `Actions` (Ctrl+W hide, Ctrl+N, F5, Ctrl+±/0, F11, Ctrl+, , Ctrl+Q), `TrayController`
  (badge composed from the 128 px PNG, tooltip, left-click toggle setting, never the only way back),
  `ThemeApplier` (Fusion + palette), `AboutDialog` (links + Copy diagnostics), `SettingsDialog`
  (4 tabs, 7 options, writes straight to Settings), `diagnostics` (markdown report),
  `MainWindow` ≈ 290 lines: close-to-tray/quit, start hidden, `commitDataRequest`, fullscreen
  enter/exit incl. page-requested, min-size scaled by zoom, chat prompt, crash dialog.
- Icons copied from whatsie (`src/resources/icons`), bundled via `qt_add_resources`.

**Verified**
- 11/11 tests pass offscreen (`scripts/dev-build.sh --tests`).
- Real display (KDE Wayland): tray icon shown, WhatsApp Web loads; second process with
  `--settings --new-chat "+1 555 0100"` exited 0 and the primary executed new-chat → settings →
  raise; log written to `~/.local/share/ktechpit/whatsie/logs/whatsie.log`.

**Known gaps (by design, next milestones)**
- Page-side theme (WA's own dark mode) not yet synced — M3 script `theme-preload.js`.
- Unread count is title-based until the bridge lands (M3).
- Call pop-out windows are logged, not shown (M3, FEATURES M3).
- No `.desktop` file yet → portal app-id warning and no `whatsapp://` handler registration (M4).
- Notifications tab is a placeholder (M2).

---

## 2026-08-27 — Scope decisions (owner review)

**Done**
- Owner reviewed every `FEATURES.md` row through an interactive Q&A (4 rounds). All decisions
  recorded in the Decision column; summary in "Approved scope at a glance".
- Open questions resolved as ADR-014 (app id `com.ktechpit.whatsie`), ADR-015 (app lock kept,
  hardened, M5), ADR-016 (Linux + Windows), ADR-017 (spell check: system language + on-demand).
- Notable calls: all CORE rows kept; all DROP rows confirmed; kept extras S11, S13, S14, S16,
  S20, S26, T3, T5, T6, N3, N5, N7, N10, A7, A8, A14 (setting, default off), M6, P2, P3, P4, D4;
  dropped S24 (configurable shortcuts), S27 (translations), A9 (custom CSS), M11, P11;
  LATER: S23, T8, P8, X2.
- Skeleton updated: identity constants now `ktechpit` / `com.ktechpit.whatsie`;
  `ScrollAnimatorEnabled` default off. Rebuilt, tests green.

**Next:** M1 (usable shell) can start — no blockers.

---

## 2026-08-27 — M0 Foundation

**Done**
- Surveyed `../whatsie` (≈6 k LOC) and `../whatly` (≈35 k LOC); reports in `reference/`.
- Pulled all GitHub issues/PRs for both repos (297 + 39 issues, 40 + 61 PRs) → `reference/github-issues-2026-08-27.md`.
- Wrote `CODING_STANDARDS.md`, `LESSONS.md`, `DECISIONS.md` (ADR-001…013), `FEATURES.md`
  (≈110 rows with recommendations, awaiting owner decisions), `ROADMAP.md`.
- Scaffold: top-level CMake (Qt 6.11, C++20), `cmake/Warnings.cmake`, `cmake/Version.cmake`,
  `cmake/SnapSdkWorkaround.cmake`, `.clang-format`, `.clang-tidy`, `.editorconfig`, `.gitignore`.
- Layered targets: `whatsie_core` (logging, `Settings` facade + keys), `whatsie_platform`
  (`describeHost`, `isSandboxed`), `whatsie_web` (`WebProfile`, `WebPage` console→log,
  `WebView`, `sanitizeUserAgent`), `whatsie_ui` (`MainWindow`), `whatsie_app` (`Application`,
  generated `version.h`), executable `whatsie`.
- Tests: `tst_settings` (7 cases), `tst_user_agent` (4), `tst_smoke` (offscreen MainWindow with
  WebEngine). All pass.
- Dev scripts: `scripts/dev-build.sh` (kde-qt6-core24-sdk snap, Qt 6.11.1),
  `scripts/dev-run.sh` (kf6-core24 runtime; `--ctest` mode), `scripts/snap-runtime-env.sh`.
  Test environment baked into CTest properties so plain `ctest` works from the host shell.

**Verified**
- `scripts/dev-build.sh --tests`: clean build, 0 warnings, 3/3 tests pass.
- `scripts/dev-run.sh` on Wayland/KDE: window opens, WhatsApp Web login page loads, page console
  visible under `whatsie.web.js`, UA reported as Chrome/140 (Qt 6.11.1 Chromium) without the
  QtWebEngine token.

**Gotchas recorded**
- `qt_add_executable` finalizer runs `qmlimportscanner`, which fails from the SDK snap on
  non-Ubuntu hosts → `QT_QML_MODULE_NO_IMPORT_SCAN TRUE` on every executable (no QML anyway).
- `ctest` must run under the *host* environment (it links host libssl); only the test binaries
  get the runtime-snap `LD_LIBRARY_PATH`.
- CMake warns about `libGLX/libOpenGL` runtime search path conflicts with the SDK — benign.
- Portal warning "Could not register app ID …" is expected until a `.desktop` file is
  installed (M4).

**Blocked / waiting**
- Owner decisions in `FEATURES.md` — resolved the same day (see entry above).

---

## Milestone status

| Milestone | Status | Notes |
|---|---|---|
| M0 Foundation | ✅ done | 2026-08-27 |
| M1 Usable shell | ✅ done | 2026-08-27 |
| M2 Notifications | ✅ done | 2026-08-27 |
| M3 Web integration | — | |
| M4 Packaging & CI | — | |
| M5 Approved extras | — | list fixed in `ROADMAP.md` |
| M6 Windows | — | ADR-016 |
