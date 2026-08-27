# Lessons from whatsie (2021–2026) and whatly (2026)

What went wrong in the original, what the 300+ GitHub issues keep saying, and the rule each
lesson turns into for the rewrite. Sources: `reference/analysis-whatsie.md`,
`reference/analysis-whatly.md`, `reference/github-issues-2026-08-27.md`.

Legend: **W#** = keshavbhatt/whatsie issue, **Y#** = shakaran/whatly issue.

---

## A. Architecture mistakes (from the code)

| # | Mistake in whatsie | Consequence | Rule for the rewrite |
|---|---|---|---|
| A1 | `MainWindow` god class (~1 300 LOC across 4 `.cpp` files, 40+ members): window + tray + WebEngine lifecycle + notifications (3 backends) + theme + lock + downloads + IPC + logout hacks | Every feature touched the same file; regressions in one area broke another (theme fix broke logout, #323) | One controller per concern owned by the window (`TrayController`, `NotificationController`, `LockController`, …). 500-line class limit. |
| A2 | Global `QSettings` singleton hit ~150× with raw string keys; passcode key literally `asdfg`, repeated 17× | Typos, no defaults, no change signals, permissions dialog wrote `permissions/<Name>` while the page read `permissions/<int>` → dialog was inert for years | `core::Settings` facade: typed accessors, one defaults table, change signals. `QSettings` constructed nowhere else. |
| A3 | `SettingsWidget` was model + view + controller (owned the auto-theme timer, wrote QSettings in every slot, MainWindow emitted *its* signals) | Inverted ownership; impossible to test | UI reacts to `Settings` signals. Services (theme scheduler, auto-lock) live in `core/`, not in dialogs. |
| A4 | Profile singleton destroyed after `QApplication` | Exit-time warnings/crashes | Profile is a `QObject` child of the view/app; destruction order guaranteed. |
| A5 | JS injected with `runJavaScript()` on every `loadFinished`, built from C++ string literals; theme script depends on WA private modules + React fiber internals + DOM classes | "Theme reverts to light" fixed four times (W#24, 62, 206, 299, 317, 332); logout clicker uses `data-testid` selectors removed in 2023 | Scripts are `.js` resources, injected via the **profile-level** script collection, each with try/catch + failure report over `QWebChannel`, header comment with last-verified date. Never depend on WA internals for anything the app can do from C++ instead. |
| A6 | "Loading quirk" watchdog: after 4 retries or any title containing "Error" it **wipes cache + storage, resets UA, and quits** | Users logged out for no reason; looks like a "sync bug" (~30 issues in cluster 1) | No self-healing that destroys state. On repeated load failure: show a diagnostic page with "reload", "clear cache", "clear session (logs out)" as *separate user actions*. |
| A7 | Hard-coded Chrome/125 UA per OS; real Chromium version from `httpUserAgent()` thrown away | "Unsupported browser", calls greyed out (Y#97), Facebook page (W#315), 429s | UA = engine default with `QtWebEngine/x` stripped (real Chromium version travels automatically). Optional override in settings. |
| A8 | `--no-sandbox` in Linux release builds | Security regression for zero benefit | Sandbox on. Snap uses `browser-support` with `allow-sandbox: true`. |
| A9 | `NotificationPopup` leaked per notification; new page created per UA reload with `downloadRequested` reconnected each time | Memory growth, N handlers per download | Rule-of-zero ownership (CODING_STANDARDS §4); connections made once in constructors. |
| A10 | Lock screen is a widget overlay; page, notifications and badge keep running; passcode stored base64 under `asdfg`; lock bypassable via notification reply (Y#41); network calls (MoreApps ads) on the lock screen | Not a lock at all | If lock is kept: hash + salt (PBKDF2/Argon2 via QPasswordDigestor), lock state enforced in the *command layer* (notifications suppressed, page hidden or unloaded), no network on lock screen. |
| A11 | Blocking `exec()` dialogs inside WebEngine callbacks; async JS results read from function-local statics | Stale values, re-entrancy bugs | Callbacks emit signals; UI answers asynchronously; no `processEvents()`. |
| A12 | No tests, no Linux CI compiling the code, no logging (JS console discarded) | Every regression found by users | CTest from day one, offscreen smoke test, `whatsie.*` logging categories, JS console routed to logs, `--log-file`. |
| A13 | Vendored GPL `libnotify-qt` inside an MIT app; vendored `SingleApplication` copy | License conflict; unmaintained fork | Freedesktop notifications via `QDBus` directly (MIT-clean); single-instance via `QLocalServer` (small, ours) or pinned FetchContent. |
| A14 | Advertised features that did not exist (spell check: 59 MB of dead dictionaries + 250 flag icons; full-width mode; Italian i18n never loaded) | Trust erosion (W#262, 235, 304) | `FEATURES.md` is the contract: a feature is listed as shipped only when it has a test and a row. Dead code is deleted, not kept. |
| A15 | Three sources of truth for the version (CMake, appdata, changelog); `BUILD_TIMESTAMP` breaks reproducible builds | Version mismatches (W#60) | Version from `project(VERSION)` only; appdata generated; no timestamps. |
| A16 | `#ifdef Q_OS_*` sprinkled inside classes for the Windows port | Untestable branches | Platform backends behind interfaces, chosen in CMake. |
| A17 | Theme forced through WhatsApp internals (React fiber, `localStorage.theme`) — and it stopped working anyway | Fragile; WhatsApp actually follows `prefers-color-scheme` | Drive the browser's colour scheme (`QStyleHints::setColorScheme`), verified live with CDP (ADR-020). |
| A18 | "Calls unsupported" blamed on the UA string for years (hard-coded Chrome/125) | The real gate was `SharedArrayBuffer` availability | Read the page's own check (`scripts/cdp-eval.mjs`) before guessing (ADR-021). |

## B. What the issue tracker teaches (297 + 39 issues)

| # | Theme (count) | Root cause | Rule |
|---|---|---|---|
| B1 | Session stuck / can receive but can't send (~30) | Old Chromium + hard-coded UA + destructive watchdog + **zero diagnostics** | Ship diagnostics: About → "Copy debug info" (Qt/Chromium version, UA, WA build), log file, tray "Reload" and "Relink (log out)" actions. Auto-reload on network resume (`QNetworkInformation`). |
| B2 | WhatsApp Web breakage (19) | Frozen Qt5 Chromium for 2 years; cert-error modal loop | Qt 6.11 via snap/Flathub runtime keeps Chromium current; certificate errors handled once, non-modal. |
| B3 | Crashes (~30) | UI work from WebEngine callbacks; notification widget lifetime; render-process death → infinite reload dialog | Widget lifetime guarded (`QPointer`); render-process recovery with exponential back-off and a cap. |
| B4 | GPU / Wayland blank screens (11) | One global `--disable-gpu` for everyone (then blanked video calls) | Setting "Hardware acceleration: auto / on / off" (restart required); detect GPU-process failure and offer safe mode once. |
| B5 | Memory / CPU (17) | Page never unloaded; V8 old-space growth | Optional "unload page when hidden for N minutes"; memory watchdog log line; no polling scripts. |
| B6 | Notifications (30) | Presenter not a real freedesktop notification; `ActionInvoked` handled without id matching → app raised on *any* notification click (recurred 2021→2025); BGRA/RGBA avatar swap; "started minimized" nag with no opt-out | One backend on `org.freedesktop.Notifications` with id tracking; avatar encoded via `QImage::Format_RGBA8888`; no informational popups without "don't show again". Windows: `QSystemTrayIcon::showMessage`. |
| B7 | Tray across desktops (19) | Tray as the only way back to the window; index-based menu manipulation; title-based badge | Never hide the window when no tray is available (whatly got this right); menu actions held as members; symbolic + monochrome icon variants. |
| B8 | Window management (13) | Hard-coded min size not scaled by zoom; fullscreen exit broken; close veto blocks logout | Persist geometry + screen; honour `fullScreenRequested(false)`; never veto session shutdown (`QGuiApplication::commitDataRequest`). |
| B9 | Theme not persisted (11, fixed 4×) | JS racing WA's own `localStorage` and Qt palette; no single source of truth | One `Theme` setting (`System/Light/Dark`), Qt side follows `QStyleHints::colorScheme`; page theme set through WA's own `localStorage` key *before* load (profile script at DocumentCreation), verified by a test. |
| B10 | Sandbox packaging (25) | Snap plugs dropped; drag-drop passes raw host paths; missing runtime libs | Portals for file chooser; plugs listed in `packaging/README.md` and tested per release on a clean VM; dev builds use the exact runtime snap (`scripts/dev-run.sh`). |
| B11 | Build friction (18) | git-dependent version, qmake/CMake confusion, 45 MB dictionaries | Tarball builds work (`WHATSIE_GIT_REVISION` falls back to "unknown"); README build section is generated from what CI actually runs. |
| B12 | Spell check / IME (15; two most-commented issues ever) | Dictionaries missing silently; IME plugins not bundled | If spell check ships: `.bdic` for the *system language* bundled, others on demand, and a visible error when the dictionary path is empty. Bundle `fcitx5`/`ibus` Qt plugins in snap/flatpak. |
| B13 | Attachments / downloads / clipboard (18) | Re-implemented download manager incomplete; download dir setting lost | Use Chromium's download flow with a minimal UI (notification + "open folder"); download dir shown in settings and in the notification. |
| B14 | App lock (8) | See A10 | See A10. |
| B15 | Multi-account / calls (17) | Single profile + single-instance guard | Profile path is a parameter from day one (`--profile <name>`), even if the UI exposes only one. Calls need: correct UA, camera/mic permissions, GPU on, `QWebEngineDesktopMediaRequest` handled. |

## C. What whatly got right (adopt)

From `reference/analysis-whatly.md` §6.1 and §9.1. These are patterns, not code to copy — the
rewrite re-implements them under `CODING_STANDARDS.md`.

| Pattern | Where whatly did it | How we adopt it |
|---|---|---|
| Feature-module convention: `enabled()` / `scriptSource()` / `install(profile)`, idempotent by script name | ~45 modules | `web::ScriptBundle` registers named resource scripts once on the profile; toggles re-install by name. Implement the shared helper **once** (whatly copied it 13×). |
| Pure policy functions with tests (`shouldNotify`, `clampZoom`, `inviteCodeFromUrl`, `isInAppPopupUrl`, `wasFrontmostRecently`, tray badge composition) | scattered | Live in `core/`, one test each. |
| Never hide the window when there is no tray icon | PR #13 | `WindowStateController` guard. |
| Wayland-safe geometry tracking (`trackNormalGeometry`) and `commitDataRequest` handling | `mainwindow.cpp` | M1. |
| UA = engine default minus `QtWebEngine/` token | `stripQtWebEngineToken` | `web::sanitizeUserAgent` (done, tested). |
| Permissions via Qt 6.8+ `permissionRequested`, persisted per type; notifications auto-granted | `webenginepage.cpp` | M3 permission controller. |
| `createWindow` keeps `web.whatsapp.com` pop-ups (calls) in-app; desktop-media picker; native `chooseFiles` with last dir | `webenginepage.cpp` | M3. |
| Clipboard image paste rescue; drop pipeline with portal path resolution | `webview.cpp`, `drop*` | M3 (drag-drop only if KEEP). |
| Renderer crash-loop breaker; connection watchdog with strike/cap policy; SW cache recovery once | `webview.cpp`, profile script B | M1/M3 with the *policy* as a pure tested function. |
| Correct freedesktop `image-data` hint, identicon fallback, `WebEngineNotifProxy` lifetime pattern | `notify()`, `webenginenotifproxy` | M2 (own QDBus client; fix identicon hash seed). |
| `isSafeToDelete` guard before any recursive delete (after W#230 wiped a `$HOME`) | `utils.cpp` | Mandatory in the storage manager. |
| PBKDF2 passcode with stored iteration count + constant-time compare | `passlock.cpp` | If P1 is KEEP. |
| Debug log ring + Chromium stderr capture + Markdown bug-report body | `debuglog.cpp` | M1 (`--log-file`, About → Copy diagnostics). |
| `--profile` process-level multi-account (settings suffix + storage suffix + instance key) | `AppProfile` | X1 plumbing from day one. |
| Rationale comments citing issue numbers | everywhere | Keep the habit; `// see W#292`. |
| Tests: offscreen QPA, isolated settings namespace, "install every script on a real profile" test, Flatpak-manifest policy test, MSVC literal-size guard | `tests/` | Split per module; keep the ideas. |
| CI: exact Qt pin with reason, post-bundle verification, snap edge/stable channels | 13 workflows | M4, trimmed to what we ship. |

## D. What whatly over-did (avoid)

From §6.2 and §9.2 of the same report; see `FEATURES.md` for the row-by-row DROP list.

| Over-reach | Cost | Rule |
|---|---|---|
| Messaging automation & APIs (scheduled/recurring send, quick compose, `--send` CLI, send-by-name, auto-reply, local HTTP API, Meta Cloud API, webhooks) | ≈4.5 k LOC, ToS risk, page-reloading sender, unauthenticated webhook path, nested `QEventLoop` on the GUI thread | A wrapper does not automate messaging. |
| AI assistant, Ollama, LibreTranslate, chat export | ≈2 k LOC, transcripts sent to endpoints, plaintext keys | Out of scope. |
| Grid view, tab tear-off, detached windows, layout persistence, black-surface nudge | ≈1.5 k LOC, the most complex code in the repo, tiny audience | Multi-account = `--profile`; tabs only if ≤ 400 LOC and requested after v1. |
| DOM cosmetics that depend on WA internals (chat-list strip, muted-status hide in 10 languages, focus mode, HD AB flag, linked-device name patch, rail buttons, undo-send, HD auto-click) | ≈1.5 k LOC on the "fragile" tier of the ladder | Scripts may use only the stable tier: standard APIs, own CSS, structural anchors (`#pane-side`, `#main`, `footer [contenteditable]`). |
| Settings sprawl: ~150 keys, 3 `QSettings` stores, a 3 248-line `SettingsWidget` that rebuilds its `.ui` at runtime, settings *search*, ~20 restart-required options, `--restart-wait` machinery | Y#98 (hang on restart) | ≤ 25 options, 4 tabs, ≤ 2 restart-required. |
| 18 performance knobs | Users toggle blindly | One "Hardware acceleration" setting; env var for experts. |
| Automatic sunrise/sunset theme + QtPositioning | 680 LOC, DST bug, UB | Follow system theme. |
| Custom frameless title bar + resizer | 400 LOC fighting compositors | Native decorations. |
| Hand-painted dictionary delegate, spell-check focus/cycle, tray spelling submenu | 540 LOC | Plain list. |
| Seven packaging formats, several unproofed; three-phase Windows codec CI producing artefacts that never ship | Maintenance | Snap + Flatpak first (M4). |
| Legacy migration code, RateApp, MoreApps, 250 flag PNGs, `.dic_delta` files | Dead weight | A new app has no legacy. |
| 6 059-line test file with `QVERIFY(true)` coverage sweeps and `contains("token")` JS checks | False confidence | One test file per module; assert behaviour. |
