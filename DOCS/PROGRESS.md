# Progress log

Newest first. One entry per work session. Keep entries factual: what changed, what was verified,
what is blocked. Milestone status table at the bottom.

---

## 2026-08-27 — Call permissions round 2 + settings polish

**Reported:** call permissions still messy ("if the user can't allow in time it's treated denied
and they never see the dialog again this session"); note-only permission UI shows nothing and
can't be toggled; Settings → Open log folder does nothing; WhatsApp file attach opens Qt's own
dialog, not the system one; Advanced tab layout overlaps (screenshot).

**Fixed**
- **Calls (ADR-022):** camera/mic (and Notifications) pre-granted for web.whatsapp.com at
  start-up while still undecided → `navigator.permissions.query` returns granted, no prompt, no
  timeout, no "address-bar icon" modal. Verified on a fresh profile: `{camera:granted,
  microphone:granted}`.
- **Permission UI:** new `ui::PermissionList` — real Camera/Microphone/Location toggles bound to
  the permission store; replaces the note + "Reset permissions" button (and its MainWindow
  plumbing).
- **Open log folder / reveal:** `platform::openDirectory` now uses the xdg-desktop-portal
  `OpenURI` D-Bus call (no spawned child, works under confinement and in dev); QDesktopServices
  fallback. Reveal keeps FileManager1 ShowItems.
- **System file dialog:** `scripts/dev-run.sh` sets `QT_QPA_PLATFORMTHEME=xdgdesktopportal` so
  file dialogs use the portal (system) chooser — the KDE platform-theme plugin is absent from
  the runtime snap and a host `qt5ct` theme forced Qt's own dialog. Shipped snap/flatpak get the
  native theme from their runtime.
- **Advanced tab overlap:** word-wrapped notes moved out of the `QFormLayout` field column into
  the group's `QVBoxLayout` (a wrapped label in a form field mis-sizes and overlaps the next
  row).

18/18 tests pass. **Owner to confirm:** a real call, attach-document dialog, Open log folder.

---

## 2026-08-27 — Call permissions: persisted and asked once

**Reported:** theme works; "call permissions are still messy". Logs showed the camera/mic grant
was never stored (`permissions.json` held only Notifications; "reset 0 site permissions") and
WhatsApp asks for *camera* then *camera+microphone* — two different Qt permission types → two
prompts per call, again after every restart.

**Cause.** In Qt WebEngine the answer to a media `permissionRequested` is session-scoped; only
`QWebEngineProfile::queryPermission(origin, type).grant()` reaches the on-disk store.

**Fixed.** `PermissionController` now (a) answers camera/mic requests from the store when any
member of the camera/mic family was decided (denial wins), (b) persists the user's answer for
the whole family, (c) `answer()` replaces direct `grant()` in the prompt. New integration test
`tst_permissions` drives a real `getUserMedia()` through a local `wtest://` scheme: one prompt,
answer stored for all three types, reload → no second prompt; denial stored too. 18/18 pass.

---

## 2026-08-27 — Fixes from owner testing (theme, calls)

**Reported:** downloads look good; theme setting not working (and the original toggled
instantly); calls say "not available in your version of browser".

**Diagnosis** (live page via `scripts/cdp-eval.mjs`, new dev tool over Chromium remote
debugging):
- WhatsApp ignores `localStorage.theme` at boot and follows `prefers-color-scheme` live.
- Calls: `WAWebVoipGatingUtils` needs `SharedArrayBuffer`; it was `undefined` because the page
  is not cross-origin isolated and QtWebEngine lacks Chrome's origin trial. Client hints /
  UA version were red herrings (whatly Y#97 has the same root cause).

**Fixed** (ADR-020, ADR-021)
- `ThemeApplier` pushes the effective scheme into `QStyleHints::setColorScheme()` and
  `WebView::refreshColorScheme()` re-applies web preferences (WebEngine only samples the scheme
  when preferences are applied — not on platform change, not on reload). Page theme follows
  **instantly, no reload**; `theme-preload.js` removed. Verified live: KDE scheme flipped
  light/dark → WhatsApp followed within a second (`prefers-color-scheme`, body class, pane
  colour); explicit Light setting → page light at boot.
- `--enable-features=SharedArrayBuffer` always on; feature-list merging fixed
  (`--enable-features` from the user is no longer overridden). Verified: `typeof
  SharedArrayBuffer === "function"`. **Owner to confirm a real call.**

---

## 2026-08-27 — M3 part A: web integration + downloads

**Owner input at kick-off:** "I am not satisfied with the current download feature. It does
not look good nor persist." → ADR-018; M8 upgraded from "minimal UI" to a persistent history
with a proper window.

**Done** (rows A1, M1, M2, M3, M5, M7, M8, M9, M12b, P5, P6 marked `done`)
- Script bundle (ADR-006): `web::ScriptBundle` installs one DocumentCreation bootstrap on the
  profile = JSON config + `qwebchannel.js` + `bootstrap.js` + `storage-persist.js` +
  `theme-preload.js`; `web::Bridge` exposed via QWebChannel for `scriptFailed`/`log`. Scripts
  are syntax-checked and the bundle is tested on a real profile.
- Page-side theme (A1): WhatsApp's own `localStorage.theme` written before boot from the
  effective scheme; a theme change reinstalls the bundle and reloads (A3 dropped as decided).
- Permissions (M1): `web::PermissionController` policy (grant notifications/screen-share/
  clipboard/mouse-lock, deny fonts, ask camera/mic/location) → non-blocking Allow/Deny prompt;
  Settings → Privacy "Reset permissions" via `profile.listAllPermissions()`.
- Screen share (M2): `ScreenPickerDialog` over `QWebEngineDesktopMediaRequest` (screens/windows
  tabs); PipeWire capturer flag on Linux.
- Call pop-out (M3): `web::PopupWindow` hosts `window.open()` targets; first navigation decides
  (WA → stays, external → browser + close); Esc/close/fullscreen exit work (W#333).
- File chooser (M5): native dialogs with last-dir memory and MIME-derived filters. Clipboard
  paste (M7): images re-exported as `image/png` before Ctrl+V (W#33/311).
- **Downloads (M8)**: `core::DownloadModel` (persisted JSON, bounded, interrupted-on-restart),
  `file_naming` (unique names, sanitising, human sizes), `web::DownloadController` (folder or
  ask, speed sampling, cancel), `ui::DownloadsDialog` + `DownloadsDelegate` (icon, name,
  status/progress, hover actions, context menu, empty state, "Open folder", "Clear finished"),
  `ui::DownloadsHub` (auto-show, completion/failure notification → click opens file, reveal via
  `org.freedesktop.FileManager1`). Ctrl+J and tray entry. Settings: folder, ask, auto-show.
- Mute (M9): Ctrl+M / tray toggle persisted. Storage (P5): sizes computed off-thread, clear
  cache, "Log out & clear session" via a marker honoured at next start behind
  `isSafeToDelete` (W#230). Hardware acceleration (P6): Auto/On/Off → Chromium flags merged
  with the user's `QTWEBENGINE_CHROMIUM_FLAGS`.
- Tests: `tst_download_model`, `tst_storage_and_flags`, `tst_web_scripts` (+ smoke covers
  theme reload and mute). 17/17 pass.

**Verified**
- Real session: `chromium flags: … --enable-features=WebRTCPipeWireCapturer`, `installed script
  "whatsie:bootstrap" at DocumentCreation`, no `script … failed` reports, WhatsApp loads.

**Not yet verified by hand** (needs a logged-in session): dark theme actually applied by
WhatsApp from the preload key; call pop-out; screen-share picker during a call; a real download
through the new window. Please try these and report.

**Next (M3 part B):** S13 connection watchdog, S14 network-resume reload, S16 service-worker
recovery, A8 privacy blur, M6 drag-and-drop attach.

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
| M3 Web integration | ◐ part A done | part B: S13, S14, S16, A8, M6 |
| M4 Packaging & CI | — | |
| M5 Approved extras | — | list fixed in `ROADMAP.md` |
| M6 Windows | — | ADR-016 |
