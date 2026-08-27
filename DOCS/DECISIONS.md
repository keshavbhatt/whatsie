# Architecture Decision Records

Short, numbered, append-only. A superseded ADR stays here with a note.
Format: Context → Decision → Consequences.

---

## ADR-001 — Qt 6.11 minimum, built against the KDE snap SDK (2026-08-27)

**Context.** Whatsie was stuck on Qt5's Chromium 87 for two years and WhatsApp Web broke
repeatedly (issue cluster B2). The distribution channels that matter (Snap via `kde-neon-6` /
`kf6-core24`, Flathub via the KDE 6.x runtime) all ship **Qt 6.11**. The dev machine's system Qt
is 6.10.2; the `kde-qt6-core24-sdk` snap provides 6.11.1 and the `kf6-core24` content snap is the
exact runtime users get. Qt 6.11 also changed WebEngine script-injection timing (see ADR-006).

**Decision.** `find_package(Qt6 6.11 REQUIRED)`. Development builds use
`scripts/dev-build.sh` (SDK snap) and `scripts/dev-run.sh` (runtime snap) — the same recipe as
the kimidoc project. System-Qt builds are supported when the distro ships ≥ 6.11.

**Consequences.** Dev/prod parity; no Qt-version `#if`s in code; Ubuntu 24.04 users get the
snap/flatpak, not a native build. `cmake/SnapSdkWorkaround.cmake` strips the SDK's generic
`/usr/include` from imported targets on non-Ubuntu hosts.

## ADR-002 — C++20, CMake only, warnings as errors in CI (2026-08-27)

**Context.** whatsie was C++17 with a leftover qmake ecosystem and a wrapper Makefile.

**Decision.** C++20, CMake ≥ 3.21 with `qt_standard_project_setup`, no qmake, strict warning set
(`cmake/Warnings.cmake`), `WHATSIE_WERROR=ON` in CI. `QT_NO_KEYWORDS`, `QT_NO_CAST_*`,
`QT_DISABLE_DEPRECATED_UP_TO=0x061100`.

**Consequences.** `Q_SIGNALS/Q_SLOTS/Q_EMIT` everywhere; explicit string conversions
(`u"…"_s`); deprecated Qt API is a compile error.

## ADR-003 — Layered static libraries with one-way dependencies (2026-08-27)

**Context.** Lesson A1/A3: a single target let everything include everything.

**Decision.** `core` → `web`/`platform` → `ui` → `app`, each a static library
(`src/<layer>/CMakeLists.txt`). `core` links only `Qt6::Core`/`Qt6::Gui`. Tests link the layer
they test.

**Consequences.** The linker enforces the architecture; `core` is unit-testable headless.

## ADR-004 — No `.ui` files, no auto-connect slots (2026-08-27)

**Context.** whatsie had 2 800 lines of Designer XML and 35 `on_x_y()` auto-connected slots that
silently break on rename; `ui_*.h` coupled headers to generated code.

**Decision.** Widgets are built in code inside a private `setupUi()`; PMF `connect` only.

**Consequences.** Slightly more code per dialog; fully diffable, refactor-safe, testable.

## ADR-005 — One typed `core::Settings` facade (2026-08-27)

**Context.** Lesson A2.

**Decision.** `core::Settings` is the only place `QSettings` is constructed. Keys live in
`settings_keys.h`, defaults in `settings.cpp`, every setter emits a change signal. No migration
from whatsie's `org.keshavnrj.ubuntu/WhatSie.conf` (project brief: users just log in again).

**Consequences.** New settings require an accessor pair + a test; the app reacts to settings
signals instead of polling.

## ADR-006 — Injected JS as resources, profile-level injection, QWebChannel bridge (2026-08-27)

**Context.** Lesson A5. Additionally, on Qt 6.11 modifying a **page-level** script collection
right before a navigation loses `DocumentCreation` injection for that navigation (measured in
kimidoc, `docs/qtwebengine-gotchas.md` §8); profile-level collections are immune.

**Decision.** Scripts are `src/web/scripts/*.js` resources registered once on the
`QWebEngineProfile` script collection before the first load. Data flows JS→C++ through a
single `QWebChannel` object; C++→JS through a config object injected at `DocumentCreation`.
No `runJavaScript` string templating. Every script is a no-op on failure and reports
`scriptFailed(name, message)`.

**Consequences.** Breakage in WhatsApp Web shows up in the log, never as a silent regression;
scripts are testable for syntax; per-load injection races are avoided.

**Scope of DOM anchors.** Scripts stick to the stable tier — standard APIs, own CSS, structural
anchors (`#pane-side`, `#main`, `footer [contenteditable]`) — never obfuscated classes. The single
sanctioned exception is `nav-settings.js` (ADR-028), which injects one Settings button into the nav
rail because nothing else can render over the web view; it matches entries by shape, not class, and
degrades to a no-op.

## ADR-007 — User agent derived from the engine default (2026-08-27)

**Context.** Lesson A7.

**Decision.** `web::sanitizeUserAgent()` strips the `QtWebEngine/x.y.z` token from
`QWebEngineProfile::httpUserAgent()`; the real Chromium version stays. A settings override exists
for emergencies. No per-OS hard-coded UA strings.

**Consequences.** Upgrading Qt upgrades the advertised browser automatically.

## ADR-008 — Chromium sandbox on for native/flatpak; snap uses --no-sandbox (2026-08-27)

**Context.** Lesson A8 (whatsie's `--no-sandbox` in release was a regression). But keeping the
Chromium sandbox in a snap needs `browser-support` with `allow-sandbox: true`, which requires a
store snap-declaration (manual reviewer approval) to auto-connect. Owner decision (2026-08-27):
match the original whatsie snap so no reviewer declaration is needed.

**Decision.** Native and Flatpak builds keep the Chromium sandbox (we never hard-code
`--no-sandbox`). The **snap** uses plain `browser-support` and passes `--no-sandbox` via the app
`environment` (`QTWEBENGINE_CHROMIUM_FLAGS`), which `core::mergeChromiumFlags` preserves —
isolation there comes from strict snap confinement (AppArmor + seccomp). No allow-sandbox, no
store declaration. Users can still override `QTWEBENGINE_CHROMIUM_FLAGS`.

## ADR-009 — Notifications over `org.freedesktop.Notifications` via QDBus, no libnotify-qt (2026-08-27)

**Context.** Lesson A13 (GPL library in an MIT app) and B6 (id-less `ActionInvoked` handling
raised the app on any notification click for four years).

**Decision.** A small `platform::linux::FreedesktopNotifier` using `QDBusInterface`, tracking
notification ids, matching `ActionInvoked`/`NotificationClosed` by id, avatars encoded as
`image-data` in `RGBA8888`. Windows: `QSystemTrayIcon::showMessage`. macOS: later.
Interface `core::INotifier`.

## ADR-010 — Single-instance via `QLocalServer`, no vendored SingleApplication (2026-08-27)

**Context.** Lesson A13; also multi-profile (B15) needs the instance key to include the profile.

**Decision.** `app::SingleInstance` (~100 lines): `QLockFile` + `QLocalServer` named
`whatsie-<profile>`; secondary instance sends a JSON command (`{"cmd":"raise"}`,
`{"cmd":"open","url":…}`) and exits. Profile name is a CLI option from day one.

## ADR-011 — No destructive self-healing (2026-08-27)

**Context.** Lesson A6.

**Decision.** On repeated load failure the app shows a diagnostic page with separate,
explicit user actions (Reload / Clear cache / Log out & clear session). Nothing is deleted
without a click.

## ADR-012 — Tests and logging are not optional (2026-08-27)

**Context.** Lesson A12, B1.

**Decision.** CTest with unit tests for `core`/`web` logic and an offscreen smoke test; logging
categories `whatsie.<layer>[.sub]`; page console routed to `whatsie.web.js`; `--log-file` CLI
option (M1). CI builds Linux with `WHATSIE_WERROR=ON` and runs the suite.

## ADR-013 — Feature scope is gated by `FEATURES.md` (2026-08-27)

**Context.** whatly's feature creep; the project owner wants to approve the list.

**Decision.** A feature is implemented only when its row in `DOCS/FEATURES.md` is marked
**Approved**. Rows are proposed by the analysis, decided by the owner.

## ADR-014 — Application id `com.ktechpit.whatsie` (owner, 2026-08-27)

**Context.** whatsie's desktop/AppStream id was `com.ktechpit.whatsie` while its settings lived
under `org.keshavnrj.ubuntu/WhatSie`. Store continuity (Flathub, snap) favours the existing id.

**Decision.** Desktop file / AppStream / snap / flatpak id: `com.ktechpit.whatsie`.
`organizationName = "ktechpit"`, `organizationDomain = "ktechpit.com"`, `applicationName =
"whatsie"` → settings at `~/.config/ktechpit/whatsie.conf`, data under
`~/.local/share/ktechpit/whatsie/`. No migration from the old settings path (ADR-005).

## ADR-015 — App lock kept, hardened, in M5 (owner, 2026-08-27)

**Decision.** Passcode hashed with PBKDF2 (`QPasswordDigestor`, stored iteration count, constant
-time compare). Lock is an application-level mode: every window covered, notifications
suppressed, page hidden (or unloaded), IPC commands other than `raise` refused, attempt
throttling. Modes: lock on start, on hide-to-tray, after idle. Changing the passcode never
touches the WhatsApp session.

## ADR-016 — Platforms: Linux + Windows for v1 (owner, 2026-08-27)

**Decision.** Linux (snap, flatpak) and Windows (MSI, SignPath per W#325) are release targets.
macOS is not (no maintainer/hardware). Windows differences live in `platform/windows/`
backends (notifications via tray toast, registry autostart, taskbar badge later).

## ADR-017 — Spell check: system language bundled + on-demand downloads (owner, 2026-08-27)

**Decision.** Build converts hunspell → `.bdic` for a base set and bundles the dictionary matching
the system locale; other languages are downloadable from a fixed-tag GitHub release with a
SHA-256 manifest, listed in a plain `QListWidget`. A missing dictionary is a visible error, never
silent. Windows builds ship the same mechanism.

## ADR-018 — Downloads: persistent history and a real Downloads window (owner, 2026-08-27)

**Context.** FEATURES M8 proposed "Chromium flow + minimal UI". At the start of M3 the owner
asked for more: the original download feature "does not look good neither persists".

**Decision.** `core::DownloadModel` (QAbstractListModel) persists the last 200 downloads to
`<AppData>/downloads.json` via `QSaveFile`; entries still in progress at start-up are marked
*Interrupted*. `web::DownloadController` accepts Chromium downloads into the configured folder
with unique names (never overwrites, W#110) or asks where to save; speed is sampled every
500 ms. `ui::DownloadsDialog` renders rows with a custom delegate (file icon, name, size/speed/
relative time, progress bar, hover actions Open / Show in folder / Cancel / Remove, context
menu, double-click to open), with "Open folder" and "Clear finished". Completion raises a
notification whose click opens the file; "Show in folder" uses `org.freedesktop.FileManager1`
with a folder-open fallback. Settings: folder, ask-where-to-save, show-window-on-start.

## ADR-019 — Unread count stays title-based (2026-08-27)

**Context.** T2 planned a bridge-based count from the DOM. WhatsApp's chat-list badges carry
localised `aria-label`s and obfuscated classes — the fragile tier ADR-006 forbids.

**Decision.** The unread count comes from `document.title` ("(N) WhatsApp"), which WhatsApp
maintains itself and which has been stable for years. The bridge is used for script-failure
reporting only until a stable-tier source appears.

## ADR-020 — Page theme via the application colour scheme, not localStorage (2026-08-27)

**Context.** Owner report: "theme setting is not working" and the original toggled instantly.
Live inspection (CDP) showed WhatsApp Web ignores `localStorage.theme` at boot and follows
`prefers-color-scheme` *live* — emulating the media query flipped the page instantly.

**Decision.** `ui::ThemeApplier` calls `QStyleHints::setColorScheme()` (Qt ≥ 6.8) for an explicit
Light/Dark choice and `unsetColorScheme()` for System. Qt WebEngine copies that scheme into
Blink's web preferences only when settings are applied (at start-up — verified: neither a
platform change nor a reload updates it), so `web::WebView::refreshColorScheme()` toggles a
`QWebEngineSettings` attribute after every scheme change to force a re-apply. Result: WhatsApp
(REVISED by ADR-026 — the QStyleHints override does not survive a portal/KDE platform theme, so the page is now themed by theme-control.js; the note below describes the original approach.) switches instantly, no reload, no script touches its storage (both its JS `matchMedia`
listeners and its CSS `@media` rules follow). `theme-preload.js` was removed. `ThemeService`
ignores `colorSchemeChanged` while an explicit theme is set (the signal is our own override).
A pure `matchMedia` shim was tried and rejected: it only moves the JS-driven parts.

## ADR-026 — WhatsApp theme is forced at the page level, not via QStyleHints (2026-08-27)

**Context.** ADR-020 pushed the colour scheme by calling `QStyleHints::setColorScheme()` and
re-applying web settings so Blink re-reads `prefers-color-scheme`. Owner re-test: an explicit
Light/Dark did nothing while System-follow worked. CDP proved why — with a portal/KDE QPA
platform theme active, the platform theme *manages* `QStyleHints::colorScheme()` from the desktop
and immediately overrides our app-level `setColorScheme()`, so it never reaches Blink. (The dev
run forces `QT_QPA_PLATFORMTHEME=xdgdesktopportal`; the shipped snap's KDE platform theme does the
same.) CDP also showed WhatsApp's dark styling is keyed on a **`dark` class on `<body>`** (removing
it themes the page instantly, and WA does not re-add it), set from its own `matchMedia` read.

**Decision.** Drive WhatsApp's own theme state from `theme-control.js` (a DocumentCreation
bootstrap script), reusing the sequence proven in the original whatsie: (1) its `require()`
modules — `WAWebUserPrefsGeneral.setSystemThemeMode`/`setTheme`, `WAWebThemeContext.applyThemeToUI`,
`WAWebSystemTheme.theme`; (2) the React store — walk the `.app-wrapper-web` fiber for the component
holding `{theme, systemThemeMode}` and `setState`, else `forceUpdate` upward; (3) DOM +
`localStorage` — `data-theme`/`data-color-mode`/`style.colorScheme`, the `dark` body class, the
`theme` key and a synthetic `storage` event. Every step is individually guarded. Config arrives as
`__whatsie.config.colorScheme` (`system`/`light`/`dark`); `window.__whatsieSetTheme(mode)` applies
live changes, called from `WebView::applyThemeLive()` on `Settings::themeChanged`,
`effectiveSchemeChanged`, and every `loadFinished`. `system` sets `systemThemeMode` true so WA
follows the OS. `ThemeService`/`ThemeApplier` still theme the **Qt widgets** via `setColorScheme`;
only the web page moved to script control.

**Consequences.** Explicit Light/Dark now works regardless of platform theme, live, no reload —
verified via CDP: light/dark/system all flip `data-theme`, `body.dark` and background instantly,
including explicit Light on a Dark desktop (the case ADR-020 silently failed). An earlier attempt
that only swapped `window.matchMedia` + toggled `body.dark` also passed a direct-call CDP test but
was replaced with the WA-internal sequence because it does not update WhatsApp's React state, which
can re-derive the theme on re-render. The `ScrollAnimatorEnabled` re-push (ADR-020) stays only to
update the real `prefers-color-scheme` for System mode.

## ADR-027 — Opening a local folder uses FileManager1.ShowFolders, not portal OpenURI (2026-08-27)

**Context.** Settings → "Open log folder" did nothing, though the button reported success. Cause:
`platform::openDirectory` called the portal's `OpenURI` with a `file://` URI. `OpenURI` returns a
Request object path immediately (async) so the D-Bus reply is always a `ReplyMessage` even when the
backend refuses the request — and most backends silently no-op a bare local `file://` directory
(they expect a file descriptor).

**Decision.** `openDirectory` now tries, in order: `org.freedesktop.FileManager1.ShowFolders`
(D-Bus activated — no spawned process with a dev-broken library path; verified it opens Dolphin at
the folder), then the portal's **fd-based** `OpenDirectory` (the sanctioned route under snap/flatpak
confinement), then `portalOpenUri`, then `QDesktopServices::openUrl`. `ShowFolders` returns a typed
`QDBusReply<void>` whose `isValid()` distinguishes real success from an error. `revealInFileManager`
already used `FileManager1.ShowItems` for selecting a file; folders now match.

**Consequences.** The button works on the desktop and stays correct under confinement via the fd
path. `ShowFolders` opens the folder itself (vs `ShowItems` which selects it in the parent).

## ADR-031 — Native chrome colors are sampled from WhatsApp Web (2026-08-28)

**Context.** The Qt widgets used hand-picked "WhatsApp-ish" tokens (`#00a884`, `#111b21`, …) that
did not exactly match what WhatsApp Web renders now — a visible seam between the app chrome and the
page. Owner asked to align them. WhatsApp does not expose its theme as readable CSS variables.

**Decision.** Sample WhatsApp Web's actual rendered colors over CDP (`getComputedStyle` on `body`,
`#pane-side`, `header`, and a frequency scan for the brand green) in both themes, and set
`ThemeService`'s palettes and `whatsappStyleSheet()`'s tokens to those values. Nothing is injected
into the page — the alignment is one-directional, Qt → sampled web. Key values (2026-08-28,
WhatsApp Web 2.3000.x): accent `#21C063` dark / `#1DAA61` light (brighter in dark for contrast);
window `#262524` / `#DBD8D4`; panel `#1D1F1F` / `#FFFFFF`; text `#FAFAFA` / `#0A0A0A`.

**Consequences.** The settings dialog, tray menu and window chrome now read as the same surface as
the page (verified by screenshot, light and dark). The accent is no longer identical across themes,
so the theme-service test asserts "green-dominant" rather than "equal". If WhatsApp restyles, re-run
the sampling and update the two colour tables — they are the single source of truth.

## ADR-030 — Proxy config persists everything but the password (2026-08-28)

**Context.** FEATURES P3 wants system / none / manual (HTTP or SOCKS5) proxies with optional
authentication (M12b). The note on the row said "password via keychain or not stored"; a keychain
means a new dependency (QtKeychain) and per-desktop backends.

**Decision.** Persist `proxy/{mode,type,host,port,user}` in `QSettings`; keep the **password in
memory only** (a `Settings` member, never a key). `core::ProxyConfig` carries it for the session.
`web::applyProxy()` sets `QNetworkProxyFactory::setUseSystemConfiguration(true)` for System mode and
`QNetworkProxy::setApplicationProxy()` otherwise (QtWebEngine honours the application proxy);
`toNetworkProxy()` is a pure, tested mapping. Auth: `WebView` fills cached credentials on
`proxyAuthenticationRequired`, or (no password yet) signals the UI, which shows `ui::ProxyAuthDialog`
and stores the entered user (persisted) and password (session-only). WebRTC IP policy (P2) rides
along as the `WebRTCPublicInterfacesOnly` profile attribute, default off.

**Consequences.** No secret ever touches disk and no keychain dependency; the cost is re-entering
the proxy password once per launch. Proxy changes apply process-wide immediately; a page reload
picks them up (noted in the UI). System mode is not representable as a `QNetworkProxy`, so
`toNetworkProxy()` maps it to `DefaultProxy` and `applyProxy()` special-cases it.

## ADR-029 — Widgets follow WhatsApp Web's design system via a Qt style sheet (2026-08-28)

**Context.** Owner: the settings dialog and other widgets should match WhatsApp Web's look. The
Fusion style + WhatsApp palette (ADR-020/ThemeApplier) got the background colours right but the
controls still read as generic Fusion — square combos, default checkboxes, grey buttons.

**Decision.** `ui::whatsappStyleSheet(bool dark)` returns a Qt style sheet built from WhatsApp Web's
tokens (accent `#00a884`, panels, inputs, borders, muted text) in a light and a dark variant, and
`ThemeApplier::apply()` sets it on `qApp` on every scheme change, on top of the palette. It styles
tabs (green underline on the active one), group boxes (rounded panel cards), text inputs / combos /
spin boxes (rounded pills with chevron arrows from `:/icons/chevron-*.svg`), check boxes (green with
a white `:/icons/check.svg` tick), menus (rounded cards, green selection), scrollbars, and list
selection. Backgrounds are set only where the look needs them, so nothing paints an opaque block
over the web view. Buttons are neutral; only a `whatsiePrimary` property turns one green — never Qt's
`:default`, which followed tab order and lit up the wrong button (e.g. "Change…" over "Close").

**Consequences.** The whole widget layer (settings, about, downloads, menus, prompts) matches
WhatsApp in both themes with no per-dialog styling. Verified via screenshots, light and dark. A
single sheet is the maintenance point; the arrow/tick glyphs are the only new assets. Custom-painted
widgets (the downloads delegate) keep painting themselves; the sheet only affects their frame.

## ADR-028 — The in-page Settings button lives in WhatsApp's nav rail (2026-08-28)

**Context.** A11 wanted a Settings entry point that does not depend on a system tray. The first
attempt (ADR-024) put a `QToolButton` over the web view; it never rendered — Qt widgets do not
paint over a `QWebEngineView`, and a separate floating window cannot be positioned to track it on
Wayland (confirmed by screenshot). The only place a control can visibly sit over the web content is
*inside the page*.

**Decision.** Inject one Settings entry into WhatsApp's left nav rail from `nav-settings.js`,
adapting the original whatly's proven technique: find the rail's buttons by shape (small, far-left)
rather than by their obfuscated classes, clone a neighbouring entry so it looks native, swap in a
gear glyph, and insert it above the avatar. A click calls back through the QWebChannel `bridge`
(`Bridge::openSettings` → `settingsRequested` → `MainWindow::showSettings`). WhatsApp rebuilds the
rail on navigation, so a 1 s `setInterval` re-places the button if it is gone (whatly's lesson: a
MutationObserver over WA's constantly-mutating DOM forces layout far too often). Verified live via
CDP + screenshot: the gear appears in the rail and opening it shows the Settings dialog.

**Consequences.** This is a deliberate, *scoped* exception to ADR-006's "stable tier only" rule —
the one place fragile rail injection is justified, because there is no alternative that renders over
the web view, and it degrades gracefully (the tray and Ctrl+, still open Settings; the script is
fully guarded and never throws into the page). It is the only injected control; the multi-button
rail sprawl that ADR-006 warned about (whatly's theme/blur/zoom/strip buttons) stays dropped.

## ADR-024 — Interface scale, tray-less Settings access, and the expert flags hatch (2026-08-27)

**Context.** M5 added three things that touch startup and window chrome. (1) Interface scale
(A7) must reach Qt through `QT_SCALE_FACTOR`, which is only read during `QApplication`
construction. (2) Users without a system tray had no in-app route to Settings — the tray menu
was the only one (A11 was originally DROP because whatly injected buttons into WhatsApp's own
nav rail, which ADR-006 forbids). (3) whatsie's "remaining perf knobs" (P7) were also DROP.

**Decision.**
- **A7:** `main()` peeks at the stored `view/interfaceScale` with a bare `QSettings` *before*
  constructing `Application`, and sets `QT_SCALE_FACTOR` (unless the env var is already set).
  `Application::applyChromiumFlags()` mirrors the same factor to Chromium via
  `--force-device-scale-factor` so page rendering stays crisp. Changing it needs a restart.
  The early read targets the default profile only; a per-profile scale would need argv parsed
  in `main`, not worth it for a global appearance preference.
- **A11 (reinterpreted, DROP→KEEP; revised by ADR-028):** originally a `QToolButton` floating
  over the web view. That does not render — a Qt widget cannot paint over the `QWebEngineView`,
  and a floating top-level window cannot track it on Wayland (verified with a screenshot: the
  button was absent). Replaced by a Settings entry injected into WhatsApp's nav rail — see
  ADR-028.
- **P7 (reinterpreted, DROP→KEEP):** rather than bespoke perf toggles, we simply document that
  `mergeChromiumFlags()` already *merges* (never overwrites) a user-provided
  `QTWEBENGINE_CHROMIUM_FLAGS`, giving experts a supported escape hatch.

**Consequences.** Scale is a restart-required setting (noted in the UI). No new store
permissions. Version bumped to **6.0.0** since the original whatsie is already at 5.1.0.

## ADR-025 — Tray icon options: symbolic, hidden, connection-dimmed (2026-08-27)

**Context.** T3/T5/T6 asked for a monochrome tray icon, a way to hide the tray entirely, and a
dimmed icon while WhatsApp is disconnected.

**Decision.** `TrayController` chooses its base image from `traySymbolicIcon()` (the
`whatsie-symbolic.svg` rendered to a bitmap vs the colour PNG); honours `trayHidden()` by not
creating — or live-destroying — the `QSystemTrayIcon`; and dims via the pure, tested
`core::dimImage()` when `trayDimWhenDisconnected()` and the web layer reports the socket down
(new `WebView::connectionChanged` signal, sourced from the existing S13 watchdog bridge). All
three react to live settings changes. Because the same composed icon feeds the window/taskbar
icon, the options remain meaningful even with no system tray — which also avoids the trap of
`trayHidden` disabling the very control that un-hides it.

**Symbolic icon colour (revised 2026-08-28).** A first cut tinted the symbolic glyph to the *app*
theme's foreground; with the app on Light but the panel dark, that rendered a dark glyph on a dark
panel — invisible (owner report, verified by screenshot). The panel colour is not knowable and is
independent of the app theme, so `core::monochromeIcon()` now renders a **white glyph with a thin
dark halo** — visible on dark panels (white fill) and light ones (dark outline). No theme
dependency; `TrayController` no longer takes `ThemeService`.

## ADR-023 — Screen sharing: the portal is the picker on Wayland (2026-08-27)

**Context.** Owner testing on KDE Wayland: starting a screen share showed THREE dialogs — the
PipeWire portal's screen picker, the portal's window picker, and our own `ScreenPickerDialog` —
and clicking ours after dismissing the portal crashed the app. `QWebEngineDesktopMediaRequest`
has no validity query, so acting on a request the portal already consumed is unsafe, and a
non-modal app dialog let exactly that happen.

**Decision.** On Wayland, `MainWindow::handleDesktopMediaRequest` shows no dialog: it selects the
primary screen on the request and lets xdg-desktop-portal present its native picker and perform
the capture (verified: `getDisplayMedia` succeeds, no app dialog, no crash). On X11 there is no
portal, so `ScreenPickerDialog` remains the picker. `--enable-features=WebRTCPipeWireCapturer`
stays on (ADR-021 note / M2).

**Consequences.** One native picker on Wayland instead of three dialogs; the crash is gone.
`ScreenPickerDialog` is now X11-only. Window-specific sharing on Wayland goes through the
portal's own UI.

## ADR-022 — Camera/microphone pre-granted for WhatsApp; per-permission toggles (2026-08-27)

**Context.** Owner testing: on a video call WhatsApp shows "Allow camera and microphone — click
the icon next to the address bar" (there is no address bar). Cause: WhatsApp checks
`navigator.permissions.query` / calls `getUserMedia` the instant a call starts and treats a
slow or timed-out answer as a denial *for the whole session*, then shows that modal and never
re-asks. An async Allow/Deny prompt cannot win that race. The owner also noted the note-only
"Site permissions" UI didn't show or let them change what was granted.

**Decision.** `WebProfile` pre-grants Notifications, camera and microphone for
`https://web.whatsapp.com` at start-up, but only while the state is still *Ask*, so an explicit
user choice is never overridden. `navigator.permissions.query` then returns "granted" and calls
work with no prompt. Settings → Privacy shows a `ui::PermissionList` of real toggles (Camera,
Microphone, Location) bound to `QWebEngineProfile::queryPermission().grant()/deny()` — replacing
whatsie's inert checkbox table (LESSONS A2) and the blanket "Reset permissions" button. The OS
still gates the physical devices. Geolocation stays *Ask* (prompted on demand).

## ADR-021 — SharedArrayBuffer enabled for WhatsApp calls (2026-08-27)

**Context.** "Your browser doesn't support calling" (also whatly Y#97). WhatsApp's
`WAWebVoipGatingUtils.getUnsupportedBrowserReason()` requires `SharedArrayBuffer`, `Atomics`
and `RTCPeerConnection`. The page is not cross-origin isolated (COOP
`same-origin-allow-popups`), so Chromium only exposes SAB there through a Chrome-specific
origin trial that QtWebEngine does not honour. whatsie's hard-coded Chrome/125 UA was a
coincidence, not the cause.

**Decision.** `core::chromiumFlags()` always passes `--enable-features=SharedArrayBuffer`
(merged with `WebRTCPipeWireCapturer`); `mergeChromiumFlags()` merges feature lists because
Chromium keeps only the last `--enable-features` switch. No UA pinning (ADR-007 stands).

---

## Open questions

None open. Feature-level decisions are in `FEATURES.md` (Decision column).
