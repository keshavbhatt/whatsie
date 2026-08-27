# Progress log

Newest first. One entry per work session. Keep entries factual: what changed, what was verified,
what is blocked. Milestone status table at the bottom.

---

## 2026-08-27 — Theme (real fix) + open-log-folder

Owner: theme toggle still broken; "you already fixed it before". Honest cause: the M3 fix was
verified only via the **system** scheme flip; the **explicit** Light/Dark path never worked (the
portal/KDE platform theme overrides `QStyleHints::setColorScheme` before Blink sees it) and my
earlier check didn't exercise it. Now root-caused with CDP and fixed for real.

- **Theme:** `theme-control.js` rewritten to drive WhatsApp's own state — its `require()` theme
  modules, the React `.app-wrapper-web` store (`setState({theme, systemThemeMode})`), and DOM +
  localStorage + a synthetic storage event — the sequence proven in the original whatsie. Config
  carries `colorScheme`; `WebView::applyThemeLive()` pushes it on `themeChanged`,
  `effectiveSchemeChanged` and every `loadFinished`. **Verified live via CDP** (build 8): explicit
  Light stays light on a Dark desktop; live light/dark/system flip `data-theme`, `body.dark` and
  background instantly, no reload. Qt widgets still themed via QStyleHints. ADR-026.
- **Open log folder did nothing:** portal `OpenURI` on a local `file://` returns an async Request
  handle so the reply is always "success" while the backend no-ops it. `openDirectory` now uses
  `FileManager1.ShowFolders` (verified: opens Dolphin at the folder), then fd-based portal
  `OpenDirectory` for confinement, then fallbacks. ADR-027.
- Symbolic tray icon recolour and always-on A11 button (previous round) unchanged.

Tests 19/19. New LESSONS §E (verify the exact failing case; async portal replies aren't proof).

---

## 2026-08-27 — M5 batch A fixes: page-level theme, symbolic icon, always-on Settings button

Owner test feedback on batch A, all fixed and verified live via CDP:

- **Theme toggle regressed (explicit Light/Dark did nothing).** Root cause found with CDP: a
  portal/KDE QPA platform theme manages `QStyleHints::colorScheme()` and overrides our app-level
  `setColorScheme()`, so it never reached Blink — only System-follow worked. WhatsApp's dark
  styling is keyed on a `body.dark` class (removing it themes instantly). Fixed by theming the
  page directly: `theme-control.js` replaces `window.matchMedia` for `prefers-color-scheme` and
  toggles `body.dark`; `WebView::applyThemeLive()` pushes the mode on `themeChanged`/`loadFinished`;
  config carries `colorScheme`. Qt widgets still use QStyleHints. **Verified live**: explicit Light
  stays light on a Dark desktop; live dark/light/system flips are instant, no reload. See ADR-026.
- **Symbolic tray icon looked bad** — the SVG is a solid-black glyph, invisible on dark panels.
  Now recoloured to the effective scheme's foreground via the pure/tested `core::tintImage()`
  (light glyph in dark mode, dark in light), reacting to scheme changes.
- **A11 Settings button** now always visible (was tray-less only), per owner.

Tests 19/19 (adds `tintImage` coverage). clang-format clean. CDP method: launch with
`QTWEBENGINE_REMOTE_DEBUGGING`, drive the desktop scheme with `plasma-apply-colorscheme`, read
`matchMedia`/`body.class`/background through `scripts/cdp-eval.mjs`.

---

## 2026-08-27 — M5 batch A: autostart, shortcuts, tray options, interface scale, tray-less Settings

Version bumped **5.0.0 → 6.0.0** (original whatsie is already at 5.1.0): `project(VERSION)` and
the AppStream `<release>`.

Landed (each with a settings row + `FEATURES.md` status):
- **P4 Autostart (Linux):** `platform::setAutostartEnabled()` writes/removes
  `~/.config/autostart/com.ktechpit.whatsie.desktop` (`Exec` = snap app name under `$SNAP`, else
  `applicationFilePath()`; ` --minimized` when "start hidden" is on). `MainWindow::syncAutostart()`
  reconciles the file to the setting on launch and on change. Windows registry deferred to M6.
- **S26 Shortcuts sheet:** `ui::ShortcutsDialog` generated from `Actions::all()` (F1 action, also
  in the tray menu) — can't drift from the real `QKeySequence` bindings.
- **T3/T5/T6 Tray options:** symbolic monochrome icon (SVG→bitmap), hide-tray (live create/destroy
  of the `QSystemTrayIcon`), and connection-dim via the pure, tested `core::dimImage()` driven by
  a new `WebView::connectionChanged` signal off the S13 watchdog bridge. React to live settings
  changes; the same composed icon also feeds the window/taskbar icon. See ADR-025.
- **A7 Interface scale:** `main()` reads `view/interfaceScale` before `QApplication` and sets
  `QT_SCALE_FACTOR`; `applyChromiumFlags()` mirrors `--force-device-scale-factor`. Restart-required
  (noted in UI). Default profile only. See ADR-024.
- **A11 (DROP→KEEP, owner request):** a translucent Settings gear floating over our web-view
  widget (not DOM-injected), visible only when no system tray — so tray-less users can reach
  Settings. See ADR-024.
- **P7 (DROP→KEEP, owner request):** documented that user-set `QTWEBENGINE_CHROMIUM_FLAGS` is
  merged (never overwritten) — the supported expert perf hatch. No new code.

Verified: `scripts/dev-build.sh --tests` green (19/19; `tst_unread_badge` gains `dimImage`
coverage — desaturate+fade, zero-amount identity, null passthrough). clang-format clean.
Still pending in M5: spell check (L1/L2), proxy (P3 + auth M12b), app lock (P1, last).

---

## 2026-08-27 — M4: snap packaging (config + CI; build in the cloud)

Snap configured and building via CI (owner: no local snap builds — a snapcraft build pulls a
multi-GB KDE SDK into LXD).
- `snap/snapcraft.yaml`: core24, **kde-neon-6** extension (Qt 6.11 from `kf6-core24`), strict,
  amd64. Plugs: home, desktop(-legacy), opengl, x11, wayland, network(+bind/status/observe),
  camera, audio-playback, audio-record, removable-media, screen-inhibit-control, hardware-observe,
  unity7, and plain `browser-support`. To match the original whatsie snap and avoid a store
  reviewer declaration, the snap uses `--no-sandbox` (app `environment`, merged by
  `mergeChromiumFlags`); native/flatpak keep the real Chromium sandbox (ADR-008, owner decision).
- `dist/linux/com.ktechpit.whatsie.desktop` + `.metainfo.xml`; top-level CMake `install()` rules
  stage the binary, desktop file, metainfo and hicolor icons. `adopt-info` takes the version from
  the metainfo. Verified the install tree stages correctly into a `/usr` prefix.
- `.github/workflows/snap.yml`: `snapcore/action-build` on push → artifact, optional edge publish
  behind `PUBLISH_TO_STORE`.
- The kde-neon-6 runtime provides GPU/audio/PipeWire/portal wiring natively, so the shipped snap
  needs none of the dev-run workarounds.
- Snapcraft file trimmed to essentials; comments kept only on stage-packages for future reference.

Local snap build attempt was cancelled and the LXD build containers cleaned (recovered ~9 GB).
Flatpak deferred (owner). **Owner to confirm** the CI snap build once pushed.

---

## 2026-08-27 — M3 part B complete (watchdog, SW recovery, blur, drag-drop)

S13/S14/S16/A8 landed earlier today; **M6 drag-and-drop attach** now done: dropped local files
are read off-thread (`web::buildDropPayload`, size-capped, tested) and injected into WhatsApp as
a synthetic `drop` carrying real File objects (`file-drop.js`), which works where Chromium's
native drop can't read the paths (Wayland / Flatpak, Y#32). Verified live: the drop function is
defined and returns true for a synthetic file; unit test covers the payload builder (cap, skip
dirs/oversized, MIME).

**Build gotcha fixed:** `Qt6::Concurrent` was linked in `web` without a `find_package`, which
made the CMake *generate* step fail silently — `cmake --build` then reused stale build files, so
a just-added script (file-drop.js) wasn't in the running binary even though tests "passed".
Added Concurrent to the top-level `find_package`. Lesson: watch for "Generate step failed".

M3 part B done → M3 (Web integration) complete. Next: **M4 packaging & CI**.

---

## 2026-08-27 — Dev-run GPU (screen-share stop crash) + note on runtime-env parity

**Reported:** turning off screen share crashed the app (`EGL_BAD_DISPLAY`, "make lost context
current", `Trace/breakpoint trap`), after repeated `Failed to create GBM buffer for EGL`.

**Cause (environment, not our code).** The `kf6-core24` runtime snap ships only stub Mesa — its
`dri/` has no driver for the host GPU — so GBM allocation fails and stopping a PipeWire
screencast tears down the EGL context and aborts. The shipped snap/flatpak get real Mesa from the
`mesa-2404` content snap via the `kde-neon-6` / KDE runtime; dev-run did not wire it.

**Fixed (dev-run only).** `scripts/snap-runtime-env.sh` now points `LD_LIBRARY_PATH`,
`LIBGL_DRIVERS_PATH`, `GBM_BACKENDS_PATH`, `LIBVA_DRIVERS_PATH`, `__EGL_VENDOR_LIBRARY_DIRS`,
`DRIRC_CONFIGDIR` at `mesa-2404` (mirrors `gpu-2404-provider-wrapper`). Verified: start→stop
screencast no longer crashes; only a transient non-fatal "lost context" warning remains.

### Note: dev-run environment-parity workarounds (owner call, 2026-08-27)
None of these were application-code bugs — they are the dev-run harness lacking wiring the
shipped package gets for free from its runtime/interfaces. All live in `scripts/` only:
- **Audio** — `libpulse` needs the runtime's `pulseaudio` dir (else ALSA fallback fails).
- **PipeWire** — `SPA_PLUGIN_DIR`/`PIPEWIRE_MODULE_DIR` point at the runtime's own plugins.
- **GPU/Mesa** — `mesa-2404` provider wiring (this entry).
- **File dialogs** — `QT_QPA_PLATFORMTHEME=xdgdesktopportal` (the KDE theme plugin isn't in the
  runtime snap).
These will be validated for real in **M4** (snap/flatpak build on a clean VM). The genuine code
fixes this session were the theme mechanism (ADR-020), permission wiring/auto-grant, pop-out
media wiring, the profile-release shutdown ordering, and the screen-picker handoff (ADR-023).

---

## 2026-08-27 — Call pop-out media + PipeWire screen-share crash

**Reported:** in a call moved to its own window the camera stays off until toggled, screen share
does nothing; screen share sometimes segfaulted (`pw.loop: can't make support.system handle`).

**Causes.**
1. The pop-out window uses a separate page that never got the permission or `desktopMediaRequested`
   wiring the main view has.
2. Camera/mic did not pre-grant reliably, so the pop-out re-prompted (and the async prompt lost
   the call's race).
3. The runtime snap's libpipewire hardcodes the Debian SPA/module paths (absent on Arch), so
   `pw_loop_new()` failed and Chromium segfaulted during portal screen capture.

**Fixed.**
- `WebView::wirePopup` attaches the PermissionController and forwards `desktopMediaRequested` for
  every pop-out page, so calls in the detached window get camera and screen share. (Verified the
  wiring is not the crash source: a two-full-instance test crashes with the wiring *disabled* too
  — that is an artefact of loading two full WhatsApp apps, which a real call pop-out does not do.)
- `PermissionController` now grants camera/mic on first request (a WhatsApp client) and stores it,
  while still honouring an explicit deny from the Settings toggles; geolocation still asks.
  `PermissionList` shows camera/mic checked by default. Verified: fresh profile, `getUserMedia`
  auto-granted with zero prompts.
- `scripts/snap-runtime-env.sh` sets `SPA_PLUGIN_DIR`/`PIPEWIRE_MODULE_DIR` to the runtime snap's
  own plugins. Verified: `getDisplayMedia` returns a track, no `pw.loop` error, no segfault.

18/18 tests pass (`tst_permissions` rewritten for auto-grant + honoured deny). **Owner to
confirm:** camera + screen share inside a real "move to new window" call.

---

## 2026-08-27 — Screen sharing: hand off to the portal on Wayland (fixes crash)

**Reported:** starting a screen share showed three dialogs (two native portal pickers + our own)
and interacting with ours after closing the portal crashed the app.

**Fixed (ADR-023).** On Wayland the PipeWire portal is the picker; `handleDesktopMediaRequest`
now selects the primary screen and shows no app dialog, so the portal's native picker drives and
performs the capture. Verified over CDP: `getDisplayMedia` returns a track, our dialog never
appears, the app survives. `ScreenPickerDialog` is now the X11-only path.

---

## 2026-08-27 — Calls: audio device fix (the real cause) + shutdown warning

**Reported:** the "Allow camera and microphone" modal still appears *after* permissions are
granted; log shows `PcmOpen: default, No such device` and `Cannot open
libasound_module_pcm_pipewire.so`, plus "Release of profile requested but WebEnginePage still
not deleted" on quit.

**Cause.** Not permissions — audio capture. The runtime snap's `libpulse.so.0` needs
`libpulsecommon-16.1.so` but reaches it only through a RUNPATH (`/usr/lib/.../pulseaudio`) that
does not exist off-snap, so libpulse failed to load and Chromium fell back to ALSA, whose
Ubuntu-built libasound looked for the PipeWire plugin in the Debian path absent on Arch. mic
open failed → `getUserMedia` rejected → WhatsApp's generic "unsupported" modal.

**Fixed.**
- `scripts/snap-runtime-env.sh` adds the runtime's `.../pulseaudio` dir to `LD_LIBRARY_PATH`, so
  libpulse loads and Chromium uses PulseAudio (pipewire-pulse). Verified via CDP:
  `getUserMedia({audio:true})` succeeds, the real mic enumerates, zero ALSA errors. (Shipping
  snap/flatpak get this from their audio interfaces; this is a dev-run parity fix.)
- `WebView::~WebView()` detaches and deletes the page and any pop-up windows before the profile
  (they are siblings under the view; QObject would otherwise free the profile first). Verified:
  a clean `--quit` no longer prints the warning.

**Owner to confirm:** a real voice/video call end to end.

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
| M3 Web integration | ✅ done | 2026-08-27 (parts A + B) |
| M4 Packaging & CI | ◐ snap done | snap config + CI; flatpak later |
| M5 Approved extras | — | list fixed in `ROADMAP.md` |
| M6 Windows | — | ADR-016 |
