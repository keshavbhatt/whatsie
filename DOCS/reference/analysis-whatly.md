# Whatly — Technical Analysis for a From-Scratch Rewrite

Project: `/home/commander/DCode/whatly` (fork of keshavbhatt/whatsie by shakaran), version 7.3.1, MIT.
Analysis date: 2026-08-27. Nothing was modified.

**Provenance in one paragraph.** Upstream WhatSie (Keshav Bhatt, 2021-04 → 2024-11, ~350 commits, Qt5/qmake, later Qt6/CMake at 5.x) was a ~8k-line single-`MainWindow` wrapper. The fork (Ángel Guzmán Maeso) added ~396 commits between **2026-07-13 and 2026-08-23 — six weeks** — with `Co-Authored-By: Claude Opus` trailers on many commits, shipping 22 releases (6.0.0 → 7.3.1). Source is now **33.8k lines of C++/headers + 4.6k of `.ui` + ~7k of tests + 1.1k vendored SingleApplication**, ~2.8k of which are JavaScript embedded in C++ raw strings. Roughly 60% of the fork's added features are automation, AI, cloud-API or cosmetic web-DOM patches that a general-audience wrapper does not need. The engineering hygiene around them, however, is unusually good for a Qt hobby project and much of it is worth copying.

---

## 1. Build system & toolchain

### CMake (`CMakeLists.txt`, 716 lines)
- `cmake_minimum_required(3.24)`, `project(whatly LANGUAGES CXX)`, `CMAKE_CXX_STANDARD 17` (required). Only `.cpp` — no C, no QML.
- **Qt 6.10 floor by default** (`QT_VERSION_MINOR` overridable to 9 for the Flatpak KDE 6.9 runtime; real API floor is 6.8 for `QWebEnginePermission` / `QWebEnginePage::permissionRequested`). The comment (CMakeLists.txt:47-56) justifies the high floor twice: the permission API, and WhatsApp Web refusing old Chromium. Debian 13/Ubuntu 24.04 (Qt 6.4) cannot build it.
- Modules: `Core Gui Widgets WebEngineWidgets WebChannel Positioning Network Svg`; optional `LinguistTools`; Linux-only `DBus` plus raw `X11`, `xcb` libs and `notify-qt` (git submodule `src/libnotify-qt` → `ahm-forks/libnotify-qt`, **not checked out in this tree**; CMake rewrites the submodule's `cmake_minimum_required(4.0)` line in-place with `file(WRITE)` at configure time — CMakeLists.txt:96-115 — a hack).
- `CMAKE_AUTOMOC/AUTORCC/AUTOUIC ON`. Sources listed explicitly (no globbing), 88 `.cpp`, 89 headers, 11 `.ui`, 1 `.qrc`.
- Hardening opt-out `WHATLY_HARDENING` (`-fstack-protector-strong -fstack-clash-protection -D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS`, `-mbranch-protection=standard` on aarch64). `-DQT_NO_DEBUG_OUTPUT` for Release. MSVC `/utf-8`.
- Build-info defines: `GIT_HASH`, `GIT_BRANCH`, `BUILD_TIMESTAMP`, `VERSIONSTR`, `WHATLY_BUILD_LABEL` (cache string appended to the startup banner), `QAPPLICATION_CLASS=QApplication` (for SingleApplication).
- Windows: `.rc` from `dist/windows/whatly.rc.in`, `WIN32_EXECUTABLE`. macOS: `MACOSX_BUNDLE` with optional `-DWHATLY_MACOS_ICNS`.
- **Spell-check dictionaries at build time** (CMakeLists.txt:560-684): `find_program(qwebengine_convert_dict)` with 8 HINT paths, FATAL_ERROR if absent (or `-DWHATLY_SPELLCHECK=OFF` → `WHATLY_NO_SPELLCHECK`). One `add_custom_command` per `src/dictionaries/*.dic` → `.bdic`; `WHATLY_DICTIONARIES` subset; `WHATLY_BUNDLE_DICTIONARIES` (default `en_US`) chooses what to *install* — the rest downloads at runtime.
- Translations: `qt_add_translations(... RESOURCE_PREFIX "/i18n")` for 22 `.ts`; WARNING (not error) without Linguist.
- Options: `WHATLY_COVERAGE` (`--coverage -O0 -g`), `WHATLY_TESTS` (adds `tests/`).
- Install (Linux): binary, `.desktop`, appdata XML, LICENSE, hicolor icons (16–256, scalable, symbolic), `.bdic` to `share/whatly/qtwebengine_dictionaries`.
- Oddities: `message(STATUS "Qt6 WebEngine spellchecker support: enabled (required)")` is a no-op placeholder; `FLATPAK_BUILD` printed in summary but never defined; `.cmake-gitignore` is a stray file.

### Third-party code
| Dep | Form | Notes |
|---|---|---|
| itay-grudev SingleApplication ≈3.3.x | vendored `src/singleapplication/` (1,095 lines) | no version pin; `userData` ctor arg used to fold `--profile` id into the instance key |
| libnotify-qt (ahm-forks) | git submodule, Linux only | falls back to system `notify-qt6` |
| Qt example code | `downloadmanagerwidget`, `downloadwidget` (BSD header from `simplebrowser`) | lightly modified |
| Identiconizer LetterTile | ported in `identicons.cpp` | |
| NOAA solar spreadsheet port | `sunclock.cpp/.hpp` | |
| hunspell dictionaries | 29 `.dic/.aff` pairs, **58 MB in-repo**, + 21 unused `.dic_delta`, + a 3-byte binary `dictionaries/urls` junk file | |
| X11/xcb | direct, for Caps-Lock detection and `XGrabKey` global hotkey | |

### Tests
QtTest. Off by default (`-DWHATLY_TESTS=ON`). Three binaries; details in §8.

### CI (`.github/workflows/`, 13 workflows)
- Qt pinned **exactly 6.10.3** everywhere via `jurplel/install-qt-action@v4` ("bump deliberately", #71).
- `tests.yml` (push/PR, ubuntu-24.04, Debug, `ctest -E settings` offscreen), `coverage.yml` (gcovr → Codecov), `windows-build.yml` (MSVC 2022, every push), `macos-build.yml` (macos-14, unsigned dmg), `release-artifacts.yml` (662 lines: create-release from CHANGELOG section → Windows exe/MSI/MSP/zip with SignPath, AppImage+zsync, fpm .deb/.rpm with hand-built qtwayland plugin and NSS modules, Flatpak bundle, openSUSE rpm in a Tumbleweed container), `snap.yml` (edge on push, stable on tag), `aur.yml` (build-test in Arch container; publish waits for AppImage sha256), `dictionaries.yml` (manual; uploads `.bdic` + manifest to fixed release tag `dictionaries`), `scorecard.yml` (OpenSSF), `windows-codecs.yml` + `windows-codecs-build.yml` (EXPERIMENTAL: rebuild QtWebEngine with proprietary codecs in three 5-hour phases with sccache — output never shipped), `release.yml`/`release-dry-run.yml` (legacy upstream go-semantic-release, dead).
- `.pre-commit-config.yaml`: conventional-commit hook (legacy).

### Packaging
| Target | Where | Status |
|---|---|---|
| Snap | `snap/snapcraft.yaml`: core24, strict, kde-neon-6 extension, amd64+arm64, 15 plugs, dictionaries converted at build | live on Snap Store |
| Debian source | `debian/` (dh 13, cmake+ninja, hardening=+all) | not built in CI (Qt too old on runners); `debian/*` oddly OSL-3.0 |
| Portable .deb/.rpm | fpm in `release-artifacts.yml`, `/opt/whatly`, bundled Qt | CI |
| Fedora RPM | `packaging/rpm/whatly.spec` | reference only |
| openSUSE/OBS | `packaging/obs/` (`WHATLY_SPELLCHECK=OFF`, no convert tool on SUSE) | CI container build |
| AUR | `packaging/aur/{whatly,whatly-bin}` | committed files stale (pkgver 6.8.0), CI stamps |
| Gentoo | `packaging/gentoo/` overlay, two identical ebuilds | "not yet proofed" |
| Flatpak | `packaging/flatpak/` (KDE 6.9 + qtwebengine BaseApp, portals, xdg-download/pictures:ro…) | CI bundle; manifest pinned `v6.0.0` (CI swaps to dir source) |
| Flathub | `packaging/flathub/` | pinned v6.1.0, not submitted |
| AppImage | `packaging/appimage/build-appimage.sh`, linuxdeploy, zsync update info | CI; in-app self-update via `appimageupdatetool` |
| Windows | WiX v4 `packaging/windows/whatly.wxs` + `patch.wxs` (MSP), SignPath signing docs | CI |
| macOS | CMake bundle + `macdeployqt -dmg` | experimental, unsigned, untested at runtime |

### `tools/` (15 scripts + a promo-video project)
`make-icons.py` (Pillow icon set gen; writes to nonexistent `debianpkg/`), `make-cards.py` (README card images; still says `whatsie-git`), `make-screenshots.sh`/`capture-main.sh`/`cdp-eval.py`/`cdp-screenshot.py` (Chrome DevTools Protocol via `--remote-debugging-port` — good pattern), `coverage.sh` (gcovr HTML), `integration.sh` (drives the real coverage-instrumented binary headless under throwaway HOME: CLI flags + IPC commands), `gen-dictionary-manifest.py`, `build-qtwebengine-codecs.sh`, `release-snap.sh` (LXD + snapcraft), `upload-store-screenshots.py` (snapcraft macaroon API), `make-video.py` (ffmpeg slideshow), `video/whatly-promo/` (a HyperFrames npm promo-video project with `CLAUDE.md`/`AGENTS.md` — does not belong in the app repo), `src/tools/find_qwebengine_convert_dict.sh` (redundant with CMake).

### Translations approach
Qt Linguist `.ts` in `src/i18n/` (22 locales, ~835 strings each, 97–99% "finished"), compiled to `.qm` and embedded under `:/i18n/`. `main.cpp:303-335` loads `qtbase_<locale>` then `:/i18n/<ll_CC>.qm` → `:/i18n/<ll>.qm`; the `language` setting overrides system locale (restart required). Language picker enumerates embedded `.qm` at runtime and labels them with `Dictionaries::languageLabel` (CLDR heuristics). **Only `it_IT` is human-translated; the other 21 are machine-generated without review** (README, docs/TRANSLATIONS.md). Upstream never actually compiled/loaded the `.ts` files — the fork fixed that.

---

## 2. Architecture

### High-level shape
Still a **`MainWindow` god object** — but split across five translation units and surrounded by ~70 small "feature modules". Pattern for feature modules (the fork's main structural idea):

```
namespace Feature {                   // pure, testable
  bool isEnabled(); void setEnabled(bool);   // QSettings accessors
  QString scriptSource();                     // JS text, no side effects
  void install(QWebEngineProfile*);           // add/replace QWebEngineScript by name
}
```
`MainWindow::initSettingWidget` (mainwindow.cpp:720-963) then wires `SettingsWidget::xChanged` → `Feature::install(profile); for each account: runJavaScript(Feature::scriptSource())`. Communication *from* the page is one `QWebChannel` object (`PageBridge`, 8 slots → 8 signals, `window.__whatlyBridge`).

Layers actually present:
1. **Bootstrap** — `main.cpp` (1,428 lines: migrations, Chromium flags, translations, 40+ CLI options, IPC dispatch, SIGTERM, Wayland→XCB relaunch).
2. **Singletons** — `SettingsManager` (per-profile `QSettings`), `WebEngineProfileManager` (one `QWebEngineProfile` per account), `Performance::settings()` (a *second* machine-wide `QSettings`), `NetworkProxy::settings()` (third), `Theme` palettes, `AppProfile` globals, `Shortcuts` registry, `DebugLog` ring buffer.
3. **Window** — `MainWindow` (5 files, 7,495 lines incl. header), `DetachedAccountWindow`, `AccountTabBar`, `CustomTitleBar`, `WindowResizer`, `Lock`, `SettingsWidget` (3,248 + 2,685 ui), dialogs.
4. **WebEngine** — `WebEnginePage`, `WebView`, `WebEngineProfileManager`, `PageBridge`, `WebEngineNotifProxy`, (dead) `RequestInterceptor`.
5. **Feature modules** — ~45 namespaces/classes (see file table).
6. **Services** — `LocalApiServer` (QTcpServer HTTP), `CloudApi`, `CloudWebhook`, `AiClient`, `OllamaManager`, `Translator`, `UpdateChecker`, `DictionaryManager`, `ScheduledMessages`, `SessionBackup`, `Backup`.

There is no dependency-injection, no interface abstraction, no controller layer; "manager" classes are singletons or MainWindow members. Global mutable state: `defaultUserAgentStr`, `defaultAppAutoLock*`, `defaultZoomFactorMaximized` (extern globals in `common.cpp`), `g_id` in `appprofile.cpp`, `DebugLog` globals, ~25 `window.__whatly*` page globals.

### File-by-file inventory (src/, lines = wc -l)

**Bootstrap / core**
| File | Lines | Responsibility |
|---|---|---|
| `main.cpp` | 1428 | Entry: legacy data migration (org rename + WhatSie→whatly), `--migrate-from`, `--unread`, Chromium flag assembly, QT_SCALE_FACTOR mirroring, Wayland RHI-failure watch + XCB relaunch, `--restart-wait` handshake, SingleApplication (profile-keyed), 45 CLI options incl. template/autoreply/cloud/localapi/webhook management, IPC `receivedMessage` dispatcher, SIGTERM socketpair, crash-recovery notice |
| `common.h/.cpp` | 87/130 | App identity constants, mutable UA global, `UnreadBreakdown`, pure helpers (`isInAppPopupUrl`, `trayTooltipText`, `inviteCodeFromUrl`, `clampZoom`) |
| `def.h` | 16 | fallback defines for GIT_HASH etc. |
| `utils.h/.cpp` | 161/727 | Static grab-bag: safe cache delete (#230), install-type detection, debug info HTML/Markdown, `/proc` RSS walk, xdg-open, random ids, pure policy predicates (`shouldArmWaylandRhiFallback`, `wasFrontmostRecently`, `topRightWithin`, `orderedByHistory<T>`) |
| `appprofile.h/.cpp` | 38/56 | `--profile=<name>` parsed from raw argv before QApplication; sanitised slug, suffix, label |
| `settingsmanager.h` | 33 | Meyers singleton over one `QSettings(org, app+suffix)` |
| `debuglog.h/.cpp` | 47/135 | 400-line ring buffer of Qt messages + page console; chained message handler; `dup2` fd-2 capture to `whatly-webengine.log` |
| `performance.h/.cpp` | 136/257 | Chromium flag builder (GPU/process/WebRTC/V8/hinting), HTTP cache, start-up crash escalation levels, suspend/unload decisions; own QSettings |
| `networkproxy.h/.cpp` | 40/79 | system/none/socks5/http proxy → `QNetworkProxy::setApplicationProxy`; own QSettings; plaintext password |
| `autostart.h/.cpp` | 24/96 | XDG autostart `.desktop` / HKCU Run; no macOS; promises `--hidden` but never passes it |
| `singleapplication/*` | 1095 | vendored |

**MainWindow (7,495 lines)**
| File | Lines | Responsibility |
|---|---|---|
| `mainwindow.h` | 740 | ~90 members; comments explain nearly every one |
| `mainwindow.cpp` | 1889 | ctor (82-316), Wayland-safe geometry tracking, zoom, theme + D-Bus portal colour-scheme, `initSettingWidget` (240 lines of wiring lambdas), notifications (libnotify `notify()`, image-data hint), close/quit, `commandSend`, `sendByNameViaWeb`, `sendAttachmentViaWeb`, `restartApp` (double-fork/exec), AppImage self-update, group invite, quick compose, rate widget |
| `mainwindow_webengine.cpp` | 2032 | `createPageFor`, `installPageBridge` (+4 page scripts: bridge, scheduled sender, attachment sender, name sender, auto-reply observer), `setNotificationPresenter` (150-line lambda, 5 backends), connection watchdog, `checkLoadedCorrectly` (wipes storage on "Error" title!), `applyPageTheme` (React-fiber hack), fullscreen, translation/export/AI/DND/reminders/data-dir glue |
| `mainwindow_accounts.cpp` | 2538 | tab strip, stack, grid view (nested splitters, mirrored columns), detached windows, tear-off/dock/absorb, dormant accounts, unread counting (IndexedDB), layout persistence with crash guard, command palette, recent-unread menu, Unity launcher badge |
| `mainwindow_tray.cpp` | 785 | 32 `QAction`s + shortcut registry, tray icon/menu, windows submenu, spelling submenu, tray click heuristics, title→unread |
| `mainwindow_lock.cpp` | 251 | lock overlay lifecycle, auto-lock filter, lock-on-hide/screensaver, change password (logs WhatsApp out!) |

**WebEngine layer**
| File | Lines | Responsibility |
|---|---|---|
| `webenginepage.h/.cpp` | 89/608 | UA re-strip, `permissionRequested` (Qt 6.8 API), desktop-capture picker dialog, external links → browser, `createWindow` for call popout, native file chooser w/ last dir, cert/auth/proxy-auth dialogs, console triage (media-stuck, SW recovery, WA loader failure), per-load JS (ctrl-wheel block, new-chat helper) |
| `webview.h/.cpp` | 80/427 | renderer-crash loop breaker + auto-restart, ctrl+wheel swallow, context menu, clipboard-image paste rescue, drag-and-drop attach with worker thread |
| `webengineprofilemanager.h/.cpp` | 49/293 | per-account profile: storage/cache paths, 13 `QWebEngineSettings` attrs, UA, spell-check, two DocumentCreation scripts (storage-persist lie, WebSocket watchdog), `applyUserSettings` reinstalls all feature scripts |
| `pagebridge.h` | 48 | QWebChannel object |
| `requestinterceptor.h` | 39 | **dead** — never installed |
| `webenginenotifproxy.h/.cpp` | 38/18 | shared_ptr over `unique_ptr<QWebEngineNotification>` with queued invoke |
| `permissiondialog.*` | 40/128/38ui | table of `QWebEnginePermission::PermissionType` |

**Accounts / window chrome**
| File | Lines | Responsibility |
|---|---|---|
| `accounttabbar.h/.cpp` | 106/314 | `QTabBar` with Chrome-style tear-off drag, sprite, insertion marker, palette tint |
| `detachedaccountwindow.h/.cpp` | 52/113 | parentless peer window holding a strip + stack |
| `customtitlebar.h/.cpp` | 61/192 | client-side title bar (standalone / merged with tabs), `startSystemMove` |
| `windowresizer.h/.cpp` | 51/95 | 8-edge resize via `startSystemResize` on 5px margin |
| `commandpalette.h/.cpp` | 48/132 | Ctrl+K fuzzy runner |
| `shortcuts.h/.cpp` | 45/70 | id/label/default registry + QSettings override + conflict check |
| `globalshortcut.h/.cpp` | 72/254 | XDG GlobalShortcuts portal, X11 `XGrabKey` fallback |
| `lingertip.h/.cpp` | 45/74 | tooltip after 5s stillness |

**Settings**
| File | Lines | Responsibility |
|---|---|---|
| `settingswidget.h/.cpp/.ui` | 344/3248/2685 | everything; ctor 780 lines rebuilds the .ui into an accordion at runtime; ~120 `on_x_y` auto-slots; 30 signals; dictionary manager UI; Ollama UI; search |
| `settingssearch.h/.cpp` | 73/204 | accent-insensitive normalise/match, layout walk |
| `setupwizard.h/.cpp` | 35/113 | 3-page first-run wizard |
| `dictionaries.h/.cpp` | 89/323 | `.bdic` dir resolution, user-dir mirroring (validates `BDic` magic), language selection/focus, `languageLabel` |
| `dictionarymanager.h/.cpp` | 87/181 | manifest fetch, download with size+SHA-256 verify, `QSaveFile` |
| `dictionaryrows.h/.cpp` | 102/309 | hand-painted delegate (arrow/spinner/trash glyphs) for the language combo popup |

**Lock**
| File | Lines | Responsibility |
|---|---|---|
| `lock.h/.cpp/.ui` | 60/272/735 | overlay widget, signup/login pages, fade, Caps-Lock via Xlib; still references dead `MoreApps` |
| `passlock.h/.cpp` | 30/68 | PBKDF2-SHA256 (210k iter, 16B salt) + legacy Base64 upgrade; key `"asdfg"` |
| `screenlock.h/.cpp` | 21/22 | `shouldLock` policy |
| `autolockeventfilter.h` | 54 | app-wide inactivity timer |
| `passworddialog.ui` | 121 | HTTP auth dialog |

**Web-DOM feature modules (JS in C++)**
| File | Lines | Feature |
|---|---|---|
| `webtweaks.h/.cpp` | 30/527 | dismiss emoji panel; 6 injected rail buttons (theme/blur/strip/zoom×3); always-on `wa_web_show_hd_photo` AB flag |
| `chatliststrip.h/.cpp` | 46/800 | collapse chat list to 97px avatar strip, unread pills, 2×2 filter grid, hover preview clone, emoji panel clamp |
| `chatnav.h/.cpp` | 64/381 | JS builders: unread lists/digest, current chat, open chat by name, focus search, **IndexedDB `model-storage/chat` unread summary** |
| `chattheme.h/.cpp` | 44/274 | 14 themes by rewriting every CSS custom property whose *value* is WA-green/neutral |
| `chatwallpaper.h/.cpp` | 33/171 | `#main` background image as base64 data URI |
| `customcss.h/.cpp` | 37/137 | user stylesheet per account |
| `customjs.h/.cpp` | 55/166 | user `.js` addons per account, MainWorld with bridge access |
| `webfont.h/.cpp` | 30/109 | font-family override |
| `hdmedia.h/.cpp` | 32/119 | auto-click HD toggle in media editor |
| `mediastuck.h/.cpp` | 53/144 | click watcher → "media never arrives" advice toast |
| `privacyblur.h/.cpp` | 34/141 | CSS blur, 5 levels |
| `focusmode.h/.cpp` | 27/87 | blur chat-list names/previews (selectors likely stale) |
| `linkeddevicename.h/.cpp` | 35/173 | patch `WAWebBrowserInfo` via `require('__debug').modulesMap` → "Whatly for Linux" + desktop icon |
| `mutedstatus.h/.cpp` | 28/186 | hide "Muted updates" by heading text in ~10 languages (has a `fromLatin1` encoding bug) |
| `undosend.h/.cpp` | 28/150 | capture Enter, countdown toast, re-dispatch |
| `quickreply.h/.cpp` | 23/38 | focus composer after notification click |
| `cannedresponses.h/.cpp` | 29/80 | saved replies + insert JS |
| `dropattach.h/.cpp` | 28/80 | JS: rebuild `File`s from base64, synthetic `paste` ClipboardEvent |
| `dropreader.h/.cpp` | 63/76 | worker-thread chunked read, 64 MiB cap, MIME sniff |
| `dropresolve.h/.cpp` | 23/65 | mime → paths, Flatpak FileTransfer portal |
| `dropprogress.h/.cpp` | 48/138 | overlay progress bar |
| `chatexport.h/.cpp` | 57/289 | JS scroll-collector → txt/json/media |
| `translator.h/.cpp` | 72/266 | LibreTranslate client + composer/selection/toast JS (toast reused app-wide) |

**Messaging / automation / APIs**
| File | Lines | Feature |
|---|---|---|
| `scheduledmessages.h/.cpp` | 100/456 | persisted queue (JSON via QSaveFile), recurrence, reminders; sender JS navigates to `/send?phone=` (full reload) |
| `scheduledmessagesdialog.*` | 38/171 | table + form |
| `messaging.h/.cpp` | 83/168 | recipient parsing, template fill, IPC encode |
| `messagetemplates.h/.cpp` | 37/77 | CLI-only `{{placeholder}}` templates |
| `autoreply.h/.cpp` | 77/198 | exact/contains/regex/hashtag rules, JSON, external rules file |
| `quickcompose.h/.cpp` | 38/94 | spotlight-style compose box (global hotkey) |
| `localapi.h/.cpp` | 108/295 | hand-rolled HTTP/1.1 on QTcpServer, loopback, bearer `/send`, un-authed `/webhook` |
| `cloudapi.h/.cpp` | 62/270 | Meta Business Cloud API (text/template/media), **blocking nested QEventLoop** |
| `cloudwebhook.h/.cpp` | 61/112 | verify handshake, HMAC (skipped if secret empty), parse |
| `aiassistant.h/.cpp` | 81/294 | OpenAI-compatible chat completions; 7 prompts; DOM transcript reader; /proc/meminfo |
| `ollama.h/.cpp` | 67/132 | detect/list/pull models with streaming progress |

**Notifications**
| File | Lines | Feature |
|---|---|---|
| `notificationpopup.h` | 169 | header-only custom popup (ScrollText marquee) |
| `notificationrules.h/.cpp` | 78/165 | DND schedule + manual, keywords, VIP, muted, inline-reply flag |
| `notificationreply.h/.cpp` | 50/152 | raw D-Bus org.freedesktop.Notifications with `inline-reply` |
| `portalnotification.h/.cpp` | 44/94 | xdg portal Notification |
| `identicons.h/.cpp` | 15/91 | letter tiles |
| `trayicon.h/.cpp` | 43/209 | pure tray image composition (mono/colour, badge to 99+, dimmed) |

**Misc / legacy**
| File | Lines | Feature |
|---|---|---|
| `about.*` | 36/187/258ui | About, prefilled GitHub bug report, 3 donate buttons |
| `rateapp.*` | 55/176/222ui | upstream nag; buggy; opens `snap://whatly` |
| `widgets/MoreApps/*` | 275/383/ui | **dead** snapcraft promo carousel, still compiled |
| `automatictheme.*` + `sunclock.*` | 45/127/110ui + 126/276 | sunrise/sunset theme via QtPositioning/geoclue |
| `theme.h/.cpp` | 27/78 | two palettes |
| `updatechecker.h/.cpp` | 64/154 | GitHub releases check, install-type advice |
| `sessionbackup.h/.cpp` | 65/222 | startup snapshot/restore of IndexedDB + Local Storage (#43) |
| `backup.h/.cpp` | 33/133 | tar.gz export/import of profile (wrong dir on Windows) |
| `storageinfo.h/.cpp` | 19/36 | dir size |
| `downloadmanagerwidget.*`, `downloadwidget.*` | ~680 | Qt example download UI |
| `widgets/elidedlabel`, `widgets/scrolltext` | 101/205 | |
| `icons.qrc` | 417 | ~100 icons + **250 unreferenced flag PNGs** + 20 store-category icons + `resize.sh` embedded as a resource |
| `certificateerrordialog.ui` | 133 | title still "Dialog" |

### What was restructured vs. a typical single-MainWindow app
- Split `MainWindow` into 5 TUs by concern (a partial-class idiom), without reducing member count.
- Extracted ~45 feature modules as **namespaces of static functions** with a pure `scriptSource()` and an `install(profile)` — the single most valuable structural decision, because it made JS generation unit-testable and made feature toggles idempotent.
- Extracted pure policy functions (`Utils::shouldArmWaylandRhiFallback`, `Performance::shouldSuspendAccount`, `NotificationRules::shouldNotify`, `SessionBackup::sessionLooksPresent`) so decisions are testable without Qt UI.
- Introduced `WebEngineProfileManager` (profiles per account) and `AppProfile` (process-level isolation).
- Kept upstream's `on_widget_signal()` auto-connect Settings monolith and grew it 4×.

---

## 3. Full feature inventory

Verdict legend: **(a)** core for a general-audience wrapper · **(b)** nice-to-have · **(c)** niche / bloat.
LOC = approximate lines dedicated to the feature (C++ + JS + UI).

### 3.1 Window, tray, lifecycle
| Feature | Implementation | LOC | Verdict | Notes |
|---|---|---|---|---|
| Persistent WhatsApp Web session in a native window | `WebEngineProfileManager`, `WebEnginePage`, `createPageFor` | 300 | **a** | |
| Single instance + IPC commands (`-w -s -l -i -t -r -n --open-scheduled`) | SingleApplication + `main.cpp:1249-1357` | 150 | **a** | argv forwarded by `split(" ")` — lossy |
| `whatsapp://` scheme handler, `send?phone=` URLs, `chat.whatsapp.com` invites | `loadSchemaUrl`, `openGroupInvite`, `triggerNewChat`, `inviteCodeFromUrl` | 90 | **a** | `triggerNewChat` interpolates unescaped text into JS |
| Tray icon with unread badge (1-9, 99+), monochrome option, connection-dim | `mainwindow_tray.cpp`, `trayicon.cpp` | 400 | **a** | badge/mono composition is pure & tested |
| Tray menu: show/hide, reload, settings, about, theme, lock, mute, quit | `createActions`, `createTrayIcon` | 200 | **a** | |
| Tray click heuristic (Windows focus grace) | `iconActivated`, `Utils::wasFrontmostRecently` | 40 | **a** | |
| Close-to-tray / quit choice, start minimized, KDE session-end honoured | `closeEvent`, `commitDataRequest` | 80 | **a** | |
| Window geometry save/restore (Wayland-safe, maximized tracked by hand) | `trackNormalGeometry`, `saveWindowGeometry` | 100 | **a** | |
| Fullscreen (F11 + page-requested) | `fullScreenRequested` | 25 | **a** | hides/shows window → flicker |
| Zoom: normal vs maximized factors, Ctrl +/-/0, clamped 0.3–3.0, min-size scaling, Ctrl+wheel suppressed | `handleZoom`, `zoomBy`, `applyMinimumSize`, injected wheel listener | 110 | **a** | |
| Light/dark Qt palette + Fusion style + WhatsApp page theme push | `Theme`, `updateWindowTheme`, `applyPageTheme` | 250 | **a** | page push uses React fiber + 3 internal modules — extremely fragile |
| Follow system theme live (xdg-desktop-portal Settings D-Bus) | `desktopColorScheme`, `onPortalSettingChanged` | 60 | **a** | |
| Automatic sunrise/sunset theme with geolocation | `automatictheme.*`, `sunclock.*`, QtPositioning | 680 | **c** | DST bug, uninitialised pointer UB, never re-computes; OS already provides prefers-color-scheme |
| Restart in place (`--restart-wait=<pid>`, double-fork exec, fd preservation) | `restartApp`, `waitForPreviousInstance` | 130 | **b** | needed only because so many settings "require restart" |
| Wayland RHI-failure → one-shot XCB relaunch (#84) | `armWaylandRhiFallback`, `relaunchInXcbIfBlank` | 60 | **b** | |
| Start-up crash escalation to safer rendering (#3) + Chromium stderr log | `Performance::evaluateStartup/armStartupWatch`, `DebugLog::captureNativeStderr` | 110 | **b** | |
| Renderer crash loop breaker + optional auto-reload | `WebView` ctor | 100 | **a** | |
| Connection watchdog (WebSocket monkey-patch, 2 strikes, 3 reloads/episode) | profile script B + `checkConnectionHealth` | 110 | **a** | solves post-suspend "Connecting…" |
| Corrupt Service-Worker cache auto-recovery (#43) | `recoverFromCorruptServiceWorker` | 25 | **b** | |
| "Error" title → wipe cache+storage+quit | `checkLoadedCorrectly` | 40 | **remove** | logs the user out on any transient error page |
| Session backup/restore of IndexedDB+LocalStorage at startup (#43) | `sessionbackup.cpp` | 290 | **b** | synchronous copy every launch; size-equality change detection; unencrypted |
| Low-disk warning + move data dir | `checkStorageSpace`, `promptChangeDataDir`, `storage/dataDir` | 90 | **b** | |
| Legacy data migration (org rename, WhatSie→whatly, `--migrate-from`) | `main.cpp:57-218` | 160 | **c** for a rewrite | fork-specific |
| `--unread` runtime file + Unity LauncherEntry badge | `runUnreadQuery`, `updateLauncherBadge` | 40 | **b** | |
| Interface font size / interface scale (QT_SCALE_FACTOR mirrored to Chromium) | `main.cpp:366-381`, `interfaceFontSize` | 30 | **b** | |
| `WHATLY_MAX_FPS` env | `main.cpp:384` | 3 | **c** | |
| Setup wizard (3 pages) | `setupwizard.*` | 150 | **b** | |
| Rate-app nag (5 launches / 5 days, 30 s) | `rateapp.*` | 450 | **c / remove** | re-arms itself after "Rate"/"Donate"; points at upstream snap |
| About + prefilled GitHub bug report with memory + log | `about.*`, `Utils::appDebugInfoMarkdown` | 480 | **a** (about) / **c** (3 donate buttons, "More apps") | |
| MoreApps snapcraft carousel | `widgets/MoreApps` | 660 | **remove** | dead code that still compiles and would phone snapcraft.io |
| Update checker (GitHub releases, daily, install-type-aware advice) | `updatechecker.*` | 220 | **b** | on by default — phones home |
| AppImage self-update via `appimageupdatetool` | `startAppImageSelfUpdate` | 100 | **c** | |
| Global hotkeys Ctrl+Alt+W raise, Ctrl+Alt+N quick compose (portal + X11) | `globalshortcut.*` | 330 | **b** | Linux only |
| Customisable local shortcuts with conflict detection (32 actions) | `shortcuts.*`, Settings | 100 | **b** | restart required |
| Command palette Ctrl+K (fuzzy over actions, accounts, saved replies) | `commandpalette.*`, `showCommandPalette` | 190 | **b** | |
| Custom frameless title bar (standalone / merged with tabs) + 8-edge resizer | `customtitlebar.*`, `windowresizer.*` | 400 | **c** | cosmetic |
| Interface translations, 22 languages, in-app picker | CMake + `main.cpp` + Settings | 100 code | **a** (mechanism) | 21 machine translations unreviewed |

### 3.2 Multi-account
| Feature | Implementation | LOC | Verdict | Notes |
|---|---|---|---|---|
| `--profile=<name>` separate process/session/settings/instance key | `AppProfile`, `SettingsManager`, SingleApplication userData | 100 | **b** | cheapest, cleanest multi-account story |
| In-window account tabs (+ add/rename/remove, hidden when single) | `buildAccountArea`, `addAccount`, `AccountTabBar` basics | 600 | **b** | |
| Chrome-style tab tear-off, detached windows, drag between windows, absorb-into-main | `AccountTabBar` drag, `DetachedAccountWindow`, movers | 900 | **c** | most complex code in the repo for a tiny audience |
| Grid view of all accounts (nested splitters, mirrored columns, scroll fallback, persisted sizes) | `mainwindow_accounts.cpp:461-811` | 350 | **c** | |
| Window layout persistence + crash guard | `saveWindowLayout`/`restoreWindowLayout` | 290 | **c** | |
| Dormant accounts (unload idle / off-screen pages) | `unloadAccount`, `suspendIdleAccounts`, `unloadOffscreenWindowAccounts` | 170 | **b** | only meaningful with multi-account |
| Reparented-view black-surface nudge | `nudgeReparentedView` | 40 | **c** | platform hack |
| Per-account unread counting via IndexedDB with muted/archived/messages options; tray tooltip breakdown | `ChatNav::unreadSummaryScript`, `countUnread`, `trayTooltipText` | 250 | **a** (count) / **b** (options) | reads WA private DB schema; DOM fallback |
| Recent-unread tray submenu, open chat by name | `refreshRecentUnread`, `openChatByName` | 150 | **b** | |
| WA version per tab tooltip | `captureAccountVersion` | 40 | **c** | |
| Detach hint dialog | | 12 | **c** | |

### 3.3 Notifications
| Feature | Implementation | LOC | Verdict | Notes |
|---|---|---|---|---|
| Native notifications via libnotify-qt with avatar/identicon image-data, click → focus chat | presenter lambda, `notify()`, `notificationImageHint`, `identicons` | 300 | **a** | identicon colour not stable across runs (`qHash` seeded) |
| Windows/macOS: `QSystemTrayIcon::showMessage` | presenter | 30 | **a** | |
| In-app custom popup with timeout + marquee | `notificationpopup.h`, `scrolltext` | 380 | **b/c** | not theme-aware, double-close bug |
| XDG portal notifications (Flatpak, "auto") | `portalnotification.*` | 140 | **a** for Flatpak | can never report failure (NoBlock); no icon, no sound |
| Inline reply from notification (KDE/GNOME `inline-reply`) | `notificationreply.*`, `ensureInlineReply` | 260 | **b** | drops sound hint; send is exact-title DOM automation |
| Notification sound (`sound-name` hint) | `notify()` | 5 | **a** | only on libnotify path |
| Disable popups toggle; popup timeout | settings | 10 | **a** | |
| Do-Not-Disturb schedule + manual (indefinite/1h/2h/morning) | `notificationrules.*`, tray DND menu | 240 | **b** | |
| Keyword highlights / VIP / muted contacts (substring) | `notificationrules.*` | 60 | **b** | "Al" mutes "Alice" |
| Quick reply (focus composer on click) | `quickreply.*` | 60 | **a** | |
| Reply reminders (1h/3h/tomorrow) | `scheduleChatReminder` via `ScheduledMessages` | 40 | **c** | |
| Notifications auto-granted; permission prompt for others; persisted | `handlePermissionRequested`, `PermissionDialog` | 170 | **a** | keyed on enum int |

### 3.4 Web content tweaks (injected)
| Feature | Implementation | LOC | Verdict | Notes |
|---|---|---|---|---|
| Self-updating UA (engine UA minus `QtWebEngine/x` token) | `stripQtWebEngineToken` (+ duplicate in `WebEnginePage`) | 30 | **a** | |
| Custom UA override in settings | `useragent` key | 40 | **b** | |
| Spell check with real `.bdic` dictionaries; multi-language; on-demand download w/ SHA-256; user dir mirroring | `dictionaries.*`, `dictionarymanager.*`, CMake | 700 + 125 | **a** (core path) / **b** (download) | |
| Spell-check "focus one language" + Ctrl+Alt+S cycle + tray submenu | `Dictionaries::nextFocus`, `rebuildSpellingMenu` | 130 | **c** | |
| Hand-painted dictionary picker delegate | `dictionaryrows.*` | 410 | **c** | |
| Privacy blur (5 levels) + sidebar button | `privacyblur.*`, `webtweaks` | 170 | **b** | pure CSS, low risk |
| Focus mode (blur chat-list names) | `focusmode.*` | 115 | **c** | selectors likely stale |
| Chat colour themes (14) | `chattheme.*` | 274 | **b/c** | value-based recolour; re-parses all stylesheets |
| Chat wallpaper | `chatwallpaper.*` | 171 | **b** | base64 baked into profile script |
| Custom CSS per account | `customcss.*` | 137 | **b** | |
| Custom JS addons per account (MainWorld, bridge access) | `customjs.*` | 166 | **c** | support burden, security surface |
| Web font override | `webfont.*` | 109 | **b** | |
| Smooth scrolling (`ScrollAnimatorEnabled`) | 1 attr | 5 | **b** | |
| Theme / blur / chat-strip / zoom buttons injected into WA nav rail | `webtweaks.cpp` | 300 | **b** | geometry-based rail detection |
| Dismiss emoji panel on outside click | `webtweaks.cpp:70-114` | 45 | **b** | |
| Always-on `wa_web_show_hd_photo` AB flag | `kHdFlagScript` | 25 | **remove** | "changes nothing visible today" |
| HD media by default (auto-click) | `hdmedia.*` | 119 | **b** | body-wide MutationObserver |
| Chat-list collapse to avatar strip (unread pills, filter grid, hover preview) | `chatliststrip.*` | 800 | **c** | pixel-exact geometry, very high maintenance |
| Hide "Muted updates" section | `mutedstatus.*` | 186 | **c** | text matching in 10 languages; encoding bug |
| "Whatly for Linux" in linked devices + desktop icon | `linkeddevicename.*` | 173 | **b** | deepest reliance on WA internals |
| Undo send (Enter only) | `undosend.*` | 150 | **b** | mouse Send bypasses; stale node hazard |
| Media-stuck advice toast | `mediastuck.*` | 144 | **b** | |
| Missing-H.264 notice once | `checkMediaCodecs` | 50 | **b** | |
| Storage-persist / StorageBuckets lie | profile script A | 40 | **a** | silences WA degraded path |
| Ctrl+wheel zoom block in page | `injectPreventScrollWheelZoomHelper` | 20 | **a** | |
| Drag-and-drop file attach (portal fallback, threaded read, 64 MiB cap, progress overlay, synthetic paste) | `drop*`, `WebView::dropFiles` | 520 | **a** | JS targeting heuristics are the fragile part |
| Clipboard image paste rescue | `WebView::pasteClipboardImage` | 45 | **a** | fixes a real Qt bug |
| Native file chooser + last dir | `WebEnginePage::chooseFiles` | 60 | **a** | |
| Screen-share picker + PipeWire flag | `handleDesktopMediaRequested`, `perf/webrtcPipeWire` | 60 | **a** | |
| Call popout kept in-app | `createWindow`, `isInAppPopupUrl` | 50 | **a** | |
| Download manager window | `download*` | 680 | **a** | Qt example; timer never started → "inf B/s"; dangling pointer |
| Mute page audio | `toggleMute` | 15 | **a** | |
| Auto-play media toggle | `PlaybackRequiresUserGesture` | 10 | **b** | semantics inverted |
| Certificate / HTTP auth / proxy auth dialogs | `webenginepage.cpp:345-446` | 100 | **b** | cert override is a foot-gun for a single-origin app; double dialog |
| Inline translation (LibreTranslate) of selection/composer | `translator.*`, `translateSelection/Composer` | 340 | **c** | toast helper is reused by others |
| Chat export (txt/json/media via scroll-scrape) | `chatexport.*`, `exportCurrentChat` | 440 | **c** | |
| AI assistant: summarise, improve, suggest, formal/friendly/shorter, unread digest; memory warnings | `aiassistant.*`, `runAssistant`, `deliverAiText` | 550 | **c** | sends transcripts to configured endpoint; key plaintext |
| Ollama detect/list/pull from Settings | `ollama.*` + Settings | 250 | **c** | a model downloader in a chat wrapper |

### 3.5 Messaging automation & APIs
| Feature | Implementation | LOC | Verdict | Notes |
|---|---|---|---|---|
| Scheduled messages (persisted, recurring, catch-up, reminders) + dialog | `scheduledmessages*` | 770 | **c** | sender **reloads the page** via `location.href=/send?phone=`; blind success after 400 ms; stale selectors; catch-up burst bug |
| Quick compose overlay (Ctrl+Alt+N) | `quickcompose.*` | 130 | **b/c** | number sends go through the reloading scheduled path |
| `--send --to --message --file --template --var --backend` CLI | `main.cpp`, `messaging.*`, `commandSend` | 400 | **c** | |
| Send by contact/group name via search automation | `sendByNameViaWeb`, `nameSenderScriptSource` | 300 | **c** | self-described "NOT yet verified live" |
| Attachment send via synthetic paste + media editor click | `sendAttachmentViaWeb`, `attachmentSenderScriptSource` | 200 | **c** | 3 MiB cap |
| Message templates CLI | `messagetemplates.*` | 110 | **c** | |
| Auto-reply rules (exact/contains/regex/hashtag), CLI, rules file | `autoreply.*`, observer script | 400 | **c** | only the *open* chat; direction by x-position; no throttle |
| Local HTTP API (`POST /send`, bearer, loopback) | `localapi.*` | 400 | **c** | unbounded buffer; fire-and-forget |
| Meta Cloud Business API send (text/template/media) | `cloudapi.*` | 270 | **c** | blocking nested QEventLoop on GUI thread, no timeout |
| Cloud webhooks (verify + HMAC + auto-reply) | `cloudwebhook.*`, `/webhook` route | 170 | **c** | unsigned POSTs accepted when secret empty |
| Saved replies (canned) | `cannedresponses.*` | 110 | **b** | |

### 3.6 Privacy, lock, performance, network
| Feature | Implementation | LOC | Verdict | Notes |
|---|---|---|---|---|
| App lock (PBKDF2 passcode), lock on start / hide / screensaver / inactivity, change password | `lock.*`, `passlock.*`, `screenlock.*`, `autolockeventfilter.h`, `mainwindow_lock.cpp` | 1400 (735 ui) | **b** | covers only main window's central widget — detached windows stay usable; no attempt throttling; changing password logs WhatsApp out; key `"asdfg"` |
| Performance knobs (GPU off/compositing/vsync/in-process/blocklist, single-process, per-site, V8 optimize-for-size, JS heap cap, cache type/size, font hinting) | `performance.*` + Settings | 400 | **b** (disable-gpu) / **c** (rest) | |
| WebRTC IP shield | `perf/webrtcShield` | 5 | **b** | |
| Network proxy (system/none/SOCKS5/HTTP + auth) | `networkproxy.*` | 120 | **b** | password plaintext |
| Autostart at login | `autostart.*` | 120 | **b** | no `--hidden`, unquoted Exec, no sandbox awareness, no macOS |
| Storage manager: sizes, clear cache (safe), clear persistent | `Utils::delete_cache`, `storageinfo` | 100 | **a** | |
| Profile export/import tar.gz | `backup.*` | 170 | **b** | wrong path on Windows; blocking |
| Safe cache deletion guard (#230) | `Utils::isSafeToDelete` | 50 | **a** | |
| Debug log ring + Chromium stderr capture + Markdown report | `debuglog.*` | 180 | **b** | |
| Hardware permissions dialog | `permissiondialog.*` | 170 | **a** | |

### Feature-creep summary
Counting LOC per verdict over the whole tree (rough): **(a) ≈ 9k**, **(b) ≈ 11k**, **(c) ≈ 12k**, of which ≈ 4.5k is messaging automation/APIs, ≈ 2k AI/translation/export, ≈ 2.5k multi-window/grid/layout, ≈ 1.5k DOM cosmetics (strip, muted-status, focus, HD flag), ≈ 1.1k dead upstream (MoreApps, RateApp), ≈ 0.7k solar theme. The comments track them as "idea #2…#10" — a brainstorm list implemented wholesale.

---

## 4. Injected JavaScript / WebEngine tricks

### 4.1 Loading mechanisms
1. **Profile-scoped `QWebEngineScript`** (`profile->scripts()`), `MainWorld`, `RunsOnSubFrames=false`, removed-by-name before insert (idempotent). Two at `DocumentCreation` (storage-persist lie, WebSocket watchdog), ~14 at `DocumentReady` (feature modules).
2. **Page-scoped `QWebEngineScript`** (`page->scripts()`) installed in `installPageBridge`: `whatly-page-bridge` (qwebchannel.js from `:/qtwebchannel/qwebchannel.js` + 50 ms poll for `qt.webChannelTransport`, DocumentCreation), `whatly-scheduled-sender`, `whatly-attachment-sender`, `whatly-name-sender`, `whatly-autoreply-observer` (all DocumentReady, all unconditional).
3. **`runJavaScript` on `loadFinished`**: ctrl-wheel blocker, `openNewChatWhatly()` helper, `MediaStuck` watcher.
4. **`runJavaScript` on demand**: theme push, unread scans, chat open, focus search, export collector, AI context reader, translation read/replace, toast, canned insert, quick reply, drop paste, clipboard paste, codec probe, `__whatlyWsState()` poll every 20 s.
5. **`sessionStorage` job hand-off** for automation (`whatlyScheduledJob`, `whatlyAttachJob`, `whatlyNameJob`) — survives the `/send?phone=` navigation.
6. **QWebChannel bridge**: one shared `QWebChannel` + `PageBridge` for *all* pages; slots `toggleTheme`, `togglePrivacyBlur`, `toggleChatListStrip`, `zoomIn/Out/Reset`, `scheduledMessageResult`, `incomingMessage`. Cannot tell which account called.

### 4.2 What relies on what (fragility ladder)
| Tier | Relies on | Scripts |
|---|---|---|
| Stable (standard APIs / own CSS) | `navigator.storage`, `WebSocket`, `caches`, own `<style>` ids, `#main`, `#pane-side`, `#side`, `[role=row]`, `footer [contenteditable]` | storage lie, watchdog, SW recovery, privacy blur, wallpaper, custom CSS, font, ctrl-wheel, canned insert, quick reply, drop paste |
| Medium (aria/data-icon, English/Spanish labels, geometry) | `[data-icon="send"|"wds-ic-send-filled"|"hd"|"scissors"|"x-alt"]`, `aria-label ~ /search|buscar/`, rail buttons `left<80`, `#expressions-panel-container`, `span[title]` exact match, bubble x-position | rail buttons, emoji dismiss, HD media, scheduled/name/attachment senders, auto-reply, undo send, focus search, unread DOM fallback, chat export |
| Fragile (Meta internals) | `window.require('WAWebABProps'|'WAWebChatCollection'|'WAWebUserPrefsGeneral'|'WAWebThemeContext'|'WAWebSystemTheme'|'__debug')`, `modulesMap['WAWebBrowserInfo'|'WAWebCompanionRegClientUtils'|'WAWebEnvironment']`, React `__reactFiber*` walk, `.app-wrapper-web`, IndexedDB `model-storage/chat` fields (`unreadCount`, `markedUnread`, `archive`, `muteExpiration`, `isAutoMuted`), `localStorage.theme` | HD AB flag, group-id title, **page theme push**, **linked-device name**, unread summary |
| Text-matching in N languages | heading text needles | muted-status hide |

### 4.3 User-agent strategy
Fallback constant Chrome/130 in `common.cpp:13`; at first profile creation the engine's real UA is taken and the `QtWebEngine/x.y.z` token removed (`webengineprofilemanager.cpp:34-47`), stored in the global `defaultUserAgentStr`; `useragent` setting overrides. `WebEnginePage` ctor re-strips with a *second* algorithm and writes it to the profile on every page construction (duplicate). "Identify as Whatly" is not a UA change — it patches `WAWebBrowserInfo` at runtime. No request interception exists (`RequestInterceptor` is dead). No headers rewritten.

### 4.4 `QWebEngineSettings` set
`AutoLoadImages, JavascriptEnabled, JavascriptCanOpenWindows, LocalStorageEnabled, LocalContentCanAccessRemoteUrls, LocalContentCanAccessFileUrls, DnsPrefetchEnabled, FullScreenSupportEnabled, SpatialNavigationEnabled, JavascriptCanPaste, JavascriptCanAccessClipboard = true; LinksIncludedInFocusChain, FocusOnNavigationEnabled = false; PlaybackRequiresUserGesture = autoPlayMedia (inverted); ScrollAnimatorEnabled = smoothScrolling`. `LocalContentCanAccess*` are unnecessary and widen the attack surface.

### 4.5 Chromium flags (`main.cpp:338-388`, `Performance::chromiumFlagFragment`)
Base: `--disable-translate --disable-extensions --disable-component-update --disable-default-apps` (+ `--no-sandbox` on Linux). Per settings: `--disable-gpu` (Linux default ON), `--disable-gpu-compositing`, `--disable-gpu-vsync`, `--in-process-gpu`, `--ignore-gpu-blocklist`, `--single-process`, `--process-per-site`, `--enable-features=WebRTCPipeWireCapturer`, `--force-webrtc-ip-handling-policy=…`, `--js-flags=--max-old-space-size=N --optimize-for-size`, `--font-render-hinting=`, `--force-device-scale-factor=` (mirrors QT_SCALE_FACTOR), `--disable-frame-rate-limit` (env), `--remote-debugging-port=9421` (debug builds). Crash-recovery levels escalate these automatically.

### 4.6 Other WebEngine mechanics
- **Profiles**: storage name `whatly<suffix>[-accountId]`, persistent path `<AppLocalData|storage/dataDir>/QtWebEngine<suffix>[-id]`, cache likewise; `AllowPersistentCookies`; cache type/size from perf settings.
- **Permissions**: Qt 6.8 `permissionRequested` → `permissions/<int>` persisted; notifications auto-granted via `queryPermission(origin, Notifications).grant()`.
- **Downloads**: `profile->downloadRequested` → `DownloadManagerWidget` (reconnected on every page creation without `UniqueConnection`).
- **Desktop capture**: `desktopMediaRequested` → modal picker over `screensModel()/windowsModel()`.
- **Cert errors**: dialog then unconditional critical box (bug).
- **Notification presenter**: re-set on every `createPageFor`, theme change, timeout change.
- **Dormant pages**: `view->setPage(nullptr); delete page` — with the important note that `view->page()` on a dormant view would lazily create a page on the *default* profile.

---

## 5. Settings storage

### Stores
1. **Per-account**: `QSettings(NativeFormat, UserScope, "shakaran", "whatly" + suffix)` → `~/.config/shakaran/whatly.conf`, `whatly-work.conf`; Windows registry `HKCU\Software\shakaran\whatly[-work]`. Chosen lazily by `SettingsManager`.
2. **Machine-wide (ignores `--profile`)**: `Performance::settings()` and `NetworkProxy::settings()` — same org/app names (so on the default profile they *share the same file* as store 1), overridable via env `WHATLY_SETTINGS_APP` for tests.
3. **Files**: `<AppData>/whatly-scheduled<suffix>.json` (QSaveFile), `<AppData>/custom<suffix>.css`, `<AppData>/jsaddons<suffix>/*.js`, `<AppData>/qtwebengine_dictionaries/*.bdic`, `<AppData>/whatly-webengine.log(.prev)`, `<dataRoot>/session-snapshots<suffix>/<account>/{current,previous}`, `$XDG_RUNTIME_DIR/whatly-unread<suffix>`, `~/.config/autostart/net.shakaran.whatly.desktop`, HKCU Run `Whatly`.
4. **Secrets in plaintext** in `.conf`: `asdfg` (passcode hash — fine), `proxy/password`, `translateApiKey`, `aiApiKey`, `cloud/accessToken`, `cloudwebhook/appSecret`, `localapi/token`.

### Complete key list (~150), grouped
- **Identity/UI**: `language`, `interfaceFontSize`, `widgetStyle`, `windowTheme` (`"light"|"dark"`, normalised at startup because older builds stored translated combo text), `followSystemTheme`, `automaticTheme`, `sunrise`, `sunset`, `settingsGeo`, `ui/settingsGeometry`, `ui/settingsScroll`, `ui/settingsSections`, `ui/settingsUiVersion`, `ui/settingsWasOpen`, `setupWizardCompleted`, `hints/detachTabShown`, `permissionPromptsFixed`, `mediaCodecNoticeShown`, `dictLocaleFetched`.
- **Window**: `normalGeometry`, `wasMaximized`, `geometry` (legacy), `zoomFactor`, `zoomFactorMaximized`, `closeButtonActionCombo`, `startMinimized`, `minimizeOnTrayIconClick`, `minimizeOnlyFocusedWindow`, `hideTrayIcon`, `monochromeTrayIcon`, `customWindowFrame`, `tabsInTitleBar`, `alwaysShowAccountTabs`, `rememberWindowLayout`, `viewMode`, `windowLayout/{present,assign,detachedGeoms,detachedActives,mainActive,gridCustomized,gridGeom,gridRows,gridCols,restoreInProgress}`.
- **Accounts**: `accounts/ids` (`__default__` token), `accounts/names`, `accounts/active`, `storage/dataDir`, `sessionBackup/enabled` (default true).
- **Web**: `useragent`, `autoPlayMedia`, `smoothScrolling`, `useNativeFileDialog`, `lastAttachmentDir`, `defaultDownloadLocation`, `autoRestartOnCrash`, `muteAudio`, `spellCheckEnabled`, `spellCheckLanguage` (legacy), `spellCheckLanguages`, `spellCheckFocus`, `permissions/<int>`, `identifyInLinkedDevices`, `hideMutedStatus`, `hdMediaDefault`, `undoSendEnabled`, `undoSendSeconds`, `privacyBlur`, `privacyBlurLast`, `focusMode`, `chatTheme`, `chatWallpaper`, `customCssEnabled`, `jsAddon/<name>/enabled`, `webFontFamily`, `chatListStrip`, `chatListStripPreviewSize`, `webtweaks/{dismissExpressionsPanel,themeToggleButton,privacyBlurButton,zoomButtons,chatListStripButton}`, `unreadCountIncludesMuted`, `unreadCountIncludesArchived`, `unreadCountCountsMessages`.
- **Notifications**: `disableNotificationPopups`, `notificationCombo`, `notificationTimeOut` (ms), `notificationSound`, `notificationBackend` (`auto|portal|libnotify`), `notif/{dndEnabled,dndStart,dndEnd,keywords,vipContacts,mutedContacts,inlineReply,manualDndIndefinite,manualDndUntil}`.
- **Lock**: `lockscreen`, `asdfg`, `appAutoLocking`, `autoLockDuration`, `lockOnHideToTray`, `lockOnScreenLock`.
- **Perf (machine-wide)**: `perf/{disableGpu,disableGpuCompositing,disableGpuVsync,inProcessGpu,ignoreGpuBlocklist,singleProcess,processPerSite,webrtcShield,webrtcPipeWire,jsMemoryLimitMb,optimizeForSize,cacheType,cacheMaxMb,fontHinting,suspendInactiveAccounts,suspendAfterMinutes,unloadOffscreenWindows,interfaceScaleFactor,startupPending,recoveryLevel}`.
- **Network (machine-wide)**: `proxy/{mode,host,port,user,password}`; `checkForUpdates`, `update/lastCheckMs`.
- **Automation/AI**: `shortcut/<id>`, `canned/titles`, `canned/texts`, `messageTemplates/names`, `messageTemplates/bodies`, `autoReply/{enabled,rulesJson,rulesFile}`, `translate{Enabled,Endpoint,ApiKey,Target}`, `ai{Enabled,Endpoint,ApiKey,Model}`, `cloud/{phoneNumberId,accessToken,apiVersion}`, `cloudwebhook/{enabled,verifyToken,appSecret}`, `localapi/{enabled,port,token}`.
- **Legacy**: `app_launched_count`, `app_install_time`, `rated_already`.

Naming is inconsistent (camelCase, `group/key`, snake_case); no central registry; defaults duplicated between loader, slot, MainWindow statics and other files (e.g. `rememberWindowLayout`, `zoomFactor` ×3).

---

## 6. Code quality

### 6.1 Done well (relative to a typical first-generation Qt app)
1. **Rationale-dense comments citing issue numbers** on nearly every non-obvious decision (#3, #14, #25, #28, #34, #43, #46, #84, #85, #186, #203, #230, #239, #285 …). Reads as an engineering log; a rewrite can mine these for "why" without re-discovering the bugs.
2. **Pure/impure split with declared test intent** in headers: `autoreply.h`, `messaging.h`, `cloudapi.h`, `localapi.h`, `updatechecker.h`, `notificationrules.h`, `sessionbackup.h`, `utils.h` predicates. Test seams are explicit (`setPathsForTesting`, `clearRegistryForTest`, env base-URL overrides).
3. **Feature module convention** (`scriptSource()` + `install(profile)` + de-dup by script name) makes toggles idempotent and live.
4. **Idempotent, re-runnable userscripts** with state on `window.__whatly*` so a settings change applies without reload; the "captured closure vs live value" bug class is documented and avoided (`webtweaks.cpp:52-67`).
5. **Structural selectors over obfuscated class names** (`#pane-side`, `[role=row]`, `data-icon`), cloning native rail buttons rather than styling from scratch; timers instead of body-wide observers where WA mutates constantly (`webtweaks.cpp:386-394`); IndexedDB cursor with throttling.
6. **Real safety fixes**: `Utils::isSafeToDelete` (#230 wiped home dirs), PBKDF2 passcode with stored iteration count + constant-time compare + transparent upgrade, `QSaveFile` atomic writes, SHA-256-verified dictionary downloads, loopback-only API with bearer token, HMAC webhook, invite-code validation.
7. **Lifecycle correctness in tricky spots**: `WebEngineNotifProxy` (shared lifetime + queued invoke), `WebView::~WebView` joins the drop thread, `refreshWindowsMenu` reuses actions to avoid deleting the sender mid-signal, `pageOf()` refuses lazy page creation on dormant views, `restoreWindowLayout` crash guard, `QPointer` guards in async JS callbacks.
8. **Crash/hang guards everywhere**: renderer crash-loop breaker, watchdog strike/cap/cooldown, `prepare()` retry budget, AB-flag poll cap, SW recovery once per page, start-up crash escalation that self-heals.
9. **Modern Qt6/C++17 idioms** in fork code: `QStringLiteral`, PMF connects with context objects, `QRandomGenerator::system()`, `QPasswordDigestor`, `QElapsedTimer`, static `QRegularExpression`, `if (init; cond)`, `enum class`, `std::unique_ptr/shared_ptr`, `QWindow::startSystemMove/Resize`, `nativeInterface<QX11Application>()`, `QJsonDocument` for JS escaping, `Qt::SkipEmptyParts`.
10. **Logging discipline**: one-line-per-event `qInfo`/`qWarning` with `whatly:` prefixes, startup build banner, ring buffer + Chromium stderr capture → bug report body.
11. **Testing**: ~360 QtTest functions, offscreen, isolated settings namespace, a 40-line loopback mock HTTP server, build-portability tests (MSVC literal cap, Flatpak manifest policy).
12. **CI/packaging**: exact Qt pin with reason, post-bundle verification steps, changelog→release notes, snap edge/stable, AUR auto-publish waiting for checksum, Scorecard.
13. Windows/macOS behind `Q_OS_*` guards without forking Linux behaviour.

### 6.2 Still problematic (concrete)

**God objects / duplication**
- `MainWindow`: 5 TUs, 7.5k lines, ~90 members, owns notifications, AI, translation, export, DND, reminders, automation, updates, restart, disk space, spell-check menus. `mainwindow.h:50-738`.
- `SettingsWidget`: 3,248 lines; ctor 780 lines (`settingswidget.cpp:96-877`) that **rips every widget out of the `.ui` grid and rebuilds an accordion at runtime** through five lambdas (427-871) — the `.ui` no longer describes the page; sections identified by index; `groupBox_8` deleted after being emptied. ~120 `on_x_y` auto-connect slots (rename a widget → silent disconnect). Key + default repeated in loader and slot (`zoomFactor` 126/3079/3091; `windowTheme` 85/1279; `appAutoLocking` 308/3033).
- `initSettingWidget` (`mainwindow.cpp:720-963`): 12 near-identical "install + for-each-account runJavaScript" lambdas; two of them (`chatWallpaperChanged` 880-885, `chatThemeChanged` 887-901) only touch `m_webEngine` → wallpaper/theme apply to the active tab only in grid view.
- **13 copies** of the `install()` boilerplate and **7 copies** of `jsStringLiteral` (`customcss.cpp:46`, `webfont.cpp:34`, `privacyblur.cpp:71`, `chatwallpaper.cpp:87`, `chatliststrip.cpp:710`, `webtweaks.cpp:434`, `linkeddevicename.cpp:145`) plus 3 JSON-array variants; only one escapes `\n`, none escape U+2028/2029. `digitsOnly` ×3, per-TU `settings()` helper ×5, composer/send-button selector sets ×5 variants (`scheduledmessages.cpp:369-380`, `cannedresponses.cpp:65`, `quickreply.cpp:17`, `mainwindow_webengine.cpp:507-524, 768-777`), unread-badge span detection ×4, pointer-press sequence ×3.
- `mainwindow_accounts.cpp`: "destroy now-empty detached window" loop ×4 (986, 1680, 1990, 2474), "count docked" ×3, `refreshAccountTabs` ≈ `refreshDetachedStrips`, two copy-pasted tab context menus (327-360, 1759-1789); grid rebuilt from scratch on any change.
- Six near-identical zoom slots (`settingswidget.cpp:3076-3138`).

**Real bugs found**
- `mutedstatus.cpp:33/161`: regex with literal U+0300–U+036F loaded via `QString::fromLatin1` → accent-stripping silently broken; Russian needle is a Latin transliteration.
- `webengineprofilemanager.cpp:257-259`: `PlaybackRequiresUserGesture = autoPlayMedia` — inverted; `webenginepage.cpp:210-221` then flips it on the *default* profile only when a camera permission arrives.
- `mainwindow.cpp:1857-1858` `triggerNewChat`: `phone`/`text` from a `whatsapp://send?text=` URL interpolated into JS **unescaped** (script injection from a deep link).
- `mainwindow_webengine.cpp:1168-1182` `checkLoadedCorrectly`: page title containing "Error" → delete cache **and persistent storage** (= log out) and quit.
- `mainwindow_webengine.cpp:147-148`: `downloadRequested` reconnected on every `createPageFor` without `Qt::UniqueConnection` → N prompts per download after N reloads.
- `webenginepage.cpp:370`: `QMessageBox::critical` shown even after the user accepted the certificate; `:451` `certificates().at(0)` unguarded.
- `webview.cpp:171-190`: `createStandardContextMenu()` leaked on the image branch.
- `downloadwidget.cpp:75`: `m_timeAdded` never `start()`ed → division by zero → "inf B/s"; `updateWidget` dereferences `m_download` without `QPointer`.
- `utils.cpp:40-53` `refreshCacheSize`: >1 GiB reported as "N B" (unit overwritten by trailing `else`).
- `utils.cpp:141`: `QRandomGenerator::securelySeeded()` per character. `utils.cpp:596-617`: `desktopOpenUrl` leaks `QProcess` and never falls back on non-zero exit.
- `scheduledmessages.cpp:288 + 222`: recurring catch-up burst (N missed days → N sends); `:242-247` timed-out send doesn't chain; `:212-215` failed recurring entries stop forever; `:421-424` blind success; `:375-377` selectors stale vs `wds-ic-send-filled`; `:451` sender navigates the whole SPA.
- `cloudapi.cpp:139-151`: nested `QEventLoop`, no timeout, called from GUI slots (`mainwindow.cpp:1199, 1304`).
- `cloudwebhook.cpp:61-62`: unsigned POST accepted when `appSecret` empty (on a public tunnel → free trigger for paid auto-replies).
- `localapi.cpp:208-217`: unbounded buffer; possible double `serviceRequest`; `localapi.cpp:166` vs `mainwindow.cpp:1313` contradiction (webhook-only mode unreachable).
- `portalnotification.cpp:64-66`: `NoBlock` call can never report failure → libnotify fallback unreachable; no icon; no sound. `notificationreply.cpp:116-126`: no `sound-name` → notification sound silently ignored on KDE (inline-reply default on).
- `identicons.cpp:48`: `qHash(QString)` is randomly seeded → contact tile colour changes every launch.
- `automatictheme.h:37`: `m_gPosInfoSrc` uninitialised + early return `:30` → destructor UB `:74`; `:23` uses `standardTimeOffset` (ignores DST). `sunclock.cpp:148` seconds formula wrong; `sunclock.hpp:103` `prepare_time` declared, never defined.
- `rateapp.cpp:137-176`: "Rate"/"Donate" call `reset()` → nag re-arms; `deleteLater()` in ctor while MainWindow keeps configuring it.
- `backup.cpp:94-99, 125`: exports `AppDataLocation` but WebEngine lives in `AppLocalDataLocation` and honours `storage/dataDir` → wrong on Windows / relocated data.
- `autostart.cpp:58-61,85`: no `--hidden` despite header; unquoted `Exec=`; no Flatpak/AppImage/snap-aware command.
- `mainwindow_lock.cpp:219-234` `isLoggedIn()`: function-static bool updated by async JS → returns previous answer. Lock overlay covers only the main window's central widget; detached windows remain usable.
- `autoreply.cpp:69-70`: `$N` substitution can re-substitute literal `$1` inside a capture. `notificationrules.cpp:82`: substring contact match.
- `undosend.cpp:135-148` installs regardless of enabled flag; only intercepts Enter; captured box may be detached.
- `settingswidget.cpp:1075-1081` `isChildOf` matches any child when `objectName` is empty.
- `lock.cpp:253` opens a new X display per Caps-Lock check; `X11/XKBlib.h` in a Qt widget; no Wayland.
- `updatechecker.cpp:136-137`: failed check recorded as done for 24 h.
- `dictionaries/urls`: 3 bytes of binary garbage; `nb-NO.dic_delta` misnamed; 21 `.dic_delta` unused.
- `icons.qrc:113` embeds a shell script; 250 flag PNGs unreferenced.

**Design smells**
- Three `QSettings` stores where two share a file on the default profile.
- Mutable extern globals (`defaultUserAgentStr`, `defaultZoomFactorMaximized` …); `Theme` returns mutable references.
- `WebEngineProfileManager` destructor `qDeleteAll(m_profiles)` at static-destruction time (after QApplication) — Qt documents profiles must die before the app; `MainWindow::~MainWindow` only `deleteLater()`s the view.
- Bidirectional coupling MainWindow ↔ SettingsWidget (`qobject_cast<MainWindow*>(parent())`, MainWindow poking checkboxes); MainWindow emits `m_settingsWidget->zoomChanged()` (`mainwindow.cpp:484`).
- `WebView` ctor walks `parent()` until `objectName().contains("MainWindow")` and `dynamic_cast`s without null check (`webview.cpp:42-46`).
- `sender()`-based dispatch in `handleLoadFinished`/`handleWebViewTitleChanged`.
- Single shared `QWebChannel`/`PageBridge` for all pages — bridge calls cannot identify the account.
- 25 sites of implicit `[=]` `this` capture (deprecated in C++20).
- All network/D-Bus/tar/copy work on the GUI thread (only `DropReader` is threaded); blocking `QDBusInterface` construction per notification.
- Magic numbers duplicated (3 MiB attachment cap ×2, 1 GB disk threshold ×2, WA background colours ×2, watchdog 20/60/90 s, rail geometry `left<80`/`right<=62`).
- `getPageTheme()` is `const`, has a static cache and writes settings from an async callback; quit waits 500 ms for it "to land".
- Over-broad `MutationObserver` on `document.body` in `hdmedia.cpp` and the always-installed auto-reply observer — the exact pattern `webtweaks.cpp:386` warns burned 40% CPU.
- Secrets plaintext; `CustomJs` runs arbitrary MainWorld JS with bridge access; `LocalContentCanAccessFileUrls/RemoteUrls` enabled for no reason; cert-error override for a single-origin app.
- Legacy remnants: `foreach`, `NULL`, Qt5 `QOverload` dance for `QProcess::finished`, `Utils` is a QObject whose dtor calls `deleteLater()` on itself, default args only in definitions, static helper under `private slots`, `TODO.md` from upstream, dead workflows, `.cmake-gitignore`.

**Over-engineering**
- Grid view + tear-off + absorb + layout persistence (~1.5k lines) for multi-account power users.
- Runtime accordion rebuild of the `.ui` instead of designing the `.ui` that way (or dropping Designer).
- Settings search (360 lines incl. translator-swapping English harvest) that exists *because* the page has ~150 options.
- Hand-painted dictionary delegate (410 lines) with hover state read from `QCursor::pos()` inside `paint()`.
- 800-line chat-list strip with pixel-exact div sweeps and `getComputedStyle` per element.
- `restartApp` double-fork/exec with fd preservation; `waitForPreviousInstance` handshake — needed only because restart-required settings proliferated.
- Three-phase Windows codec CI that produces an artifact policy says never ships; building qtwayland from source in every release run.
- Seven packaging formats, several unproofed; two RPM specs; two identical ebuilds; duplicated CHANGELOG under `docs/`.
- A 6,059-line test file with a custom `main()` and coverage-chasing suites; `tst_settings` links the whole app but is excluded from CI.

---

## 7. Known bugs / limitations (from TODO.md, CHANGELOG.md, comments)
- `TODO.md` is upstream's, every item checked; nothing fork-specific.
- `main.cpp:1336` `// TODO: invetigate the crash` on `--new-chat` IPC; `moreapps.cpp:71` the only other TODO. Otherwise limitations are prose:
  - Automation (CLI send, name sender, auto-reply) "work in progress", "NOT yet verified live", "can still break when WhatsApp Web changes its markup", ToS caveat (CHANGELOG 6.6.0, `mainwindow_webengine.cpp:434`).
  - No H.264/AAC in any shipped build (AppImage/deb/rpm/Windows/macOS); only codec-enabled system Qt (Arch, Gentoo USE) plays MP4 (`docs/MEDIA_CODECS.md`, #34, #93).
  - macOS untested at runtime; Gentoo/OBS specs unproofed; Flathub not submitted.
  - `tst_settings` aborts in CI headless.
  - Qt 6.10 floor excludes Debian/Ubuntu LTS.
  - Global shortcuts Linux only; Wayland without a portal backend → none.
  - Suspended accounts receive no messages; unloaded accounts take seconds to return.
  - Linked-device name applies only to devices linked afterwards.
  - Chromium flags/shortcuts/frame/language/JS addons all "require restart".
  - Session snapshot skipped under 512 MB free; thresholds are heuristics.
  - Windows bundles no dictionaries; Windows `QFile::link` produced `.lnk` files named `.bdic` (fixed in Unreleased).
  - Regressions recorded: 7.2.0 language-box flash, 7.3.0 blanket NVIDIA→XCB caused renderer loops (relaxed 7.3.1), 7.1.0 dictionary section "never appeared".
  - Historic: 6.0.0 fixed cache-clear deleting `$HOME` (#230), KDE logout stall, permission dialog never working, 40%-CPU MutationObserver.

---

## 8. Testing

- **Framework**: QtTest only. `tests/CMakeLists.txt` (104 lines) builds three binaries, all compiling app sources directly (no mocks/interfaces):
  - `tst_ui_assets` (202 lines, 9 tests): links about/rateapp/utils/debuglog/appprofile + `.qrc`; checks resources exist, About/Rate widgets, link buttons via `QDesktopServices::setUrlHandler` sink.
  - `tst_logic` (6,059 lines, **71 classes, ~337 tests**): links ~55 feature `.cpp` + `Qt::WebEngineCore` (+DBus on Linux); custom `main()` runs classes sequentially via `QTest::qExec`, `QStandardPaths::setTestModeEnabled(true)`, `WHATLY_SETTINGS_APP=whatly-test`, `QTWEBENGINE_CHROMIUM_FLAGS=--no-sandbox --disable-gpu`. Profile-mutating classes run last.
  - `tst_settings` (786 lines, 14 tests): links the **whole app minus `main.cpp`**, constructs the real `SettingsWidget` offscreen, auto-rejects dialogs with a 25 ms timer; **excluded from CI** (aborts headless on GitHub runners).
- **Isolation**: env-var base-URL overrides baked into production (`WHATLY_CLOUD_API_BASE`, `WHATLY_UPDATE_URL`, `WHATLY_DICT_BASE_URL`), a 40-line `MockHttpServer` (QTcpServer answering 200), ephemeral-port local API with raw `QTcpSocket` HTTP, `file://` dictionary catalogue, explicit seams (`SessionBackup::setPathsForTesting`, `Shortcuts::clearRegistryForTest`).
- **Coverage areas** (by class): utils/diagnostics (40+), injected-JS generators (~25, mostly `contains("token")`), `TstScriptInstall` (installs every module on a real `QWebEngineProfile`), `TstScriptLiterals` (scans sources for raw literals > 16380 bytes — MSVC C2026), dictionaries (20), messaging/API (50+ incl. real socket round-trips, HMAC), scheduling (12), appearance/tray (25 incl. image-level badge assertions, luma contrast), performance/proxy/autostart/portal/wizard (30), passlock/screenlock (6), backup/session/file ops (25, real `tar`), shortcuts/update/fuzzy/rules/translator/export/AI/Ollama (50), widgets (command palette nav, tab bar drag events, delegate paint), `TstFlatpakManifest` (policy: no `--filesystem=home`), settings search.
- **Running**: `cmake -B build -DWHATLY_TESTS=ON && cmake --build build --target tst_ui_assets tst_logic && QT_QPA_PLATFORM=offscreen ctest --test-dir build`. CI `tests.yml` runs `ctest -E settings`. `tools/coverage.sh` → gcovr HTML (no threshold); `coverage.yml` → Codecov. `tools/integration.sh` drives the real instrumented binary (CLI + IPC) under throwaway HOME.
- **Judgement**: The genuinely valuable tests are the pure-logic ones (parsers, rules, hashing, policies), the socket-level local API tests, image assertions for the tray, and the build-portability/policy guards. Weak: dozens of string-`contains` checks on JS text, `QVERIFY(true)` coverage sweeps (`exerciseEveryControl`, `*Coverage` classes), one 6k-line file with shared global QSettings and ordering constraints, timing-based waits. The "~90%" figure covers only linked `src/` files — MainWindow, WebEnginePage and D-Bus paths are 0%.

---

## 9. Recommendations for a from-scratch rewrite

### 9.1 Adopt (patterns, files, modules)

**Architecture patterns**
1. **Feature-module convention**: `namespace X { bool enabled(); QString scriptSource(); void install(QWebEngineProfile*); }` — but implement the shared `installUserScript(profile, name, source, enabled)` and one `jsLiteral()`/`jsJson()` helper *once*. Keep every module free of `MainWindow` includes.
2. **Pure policy functions** for anything decision-shaped (`shouldNotify`, `shouldSuspend`, `installFrom`, `sessionLooksPresent`, `wasFrontmostRecently`, `topRightWithin`, `clampZoom`, `inviteCodeFromUrl`, `isInAppPopupUrl`) with unit tests alongside.
3. **Per-account `QWebEngineProfile` manager** with explicit storage/cache paths, `AllowPersistentCookies`, spell-check languages, and the "never call `view->page()` on a dormant view" rule. Destroy profiles *before* QApplication.
4. **`AppProfile`/`--profile`** process-level multi-account (settings suffix + storage suffix + SingleApplication `userData`). This is the multi-account feature to keep; drop in-window tabs or keep only the simple tab strip.
5. **`SettingsManager` singleton** — but add a typed schema (struct of keys + defaults + group) so defaults exist once and the settings UI is generated/data-driven rather than 120 auto-slots.
6. **Single `PageBridge` QWebChannel object per page** (not shared), with the account id known on the C++ side.
7. **Rationale comments citing issues** — keep the habit.

**Concrete code worth lifting nearly verbatim** (after fixing noted bugs)
- `main.cpp`: `setChromiumFlags` structure, `quietFontFallbackWarnings`, SIGTERM socketpair, `installTranslations`, Wayland RHI watch + XCB relaunch, `normalizeWindowTheme` lesson (store enum ids, never translated text).
- `webengineprofilemanager.cpp`: profile config, `stripQtWebEngineToken`, storage-persist script, WebSocket watchdog script (+ `checkConnectionHealth` strike/cap policy).
- `webenginepage.cpp`: `permissionRequested` handling with persisted `permissions/<type>`, desktop-media picker, `createWindow` keeping `web.whatsapp.com` popups (call popout) in-app, native `chooseFiles` with last dir, console triage (`isWhatsAppLoadFailure`, `isServiceWorkerRegistrationFailure`, benign-noise filter), SW cache recovery, ctrl-wheel blocker.
- `webview.cpp`: renderer-crash loop breaker, `pasteClipboardImage`, drop pipeline (`dropresolve` portal fallback, `dropreader` worker thread, `dropattach` synthetic paste, `dropprogress`).
- `mainwindow.cpp`: `trackNormalGeometry`/`saveWindowGeometry` (Wayland-safe), `applyMinimumSize`, `desktopColorScheme` portal D-Bus + `onPortalSettingChanged`, `notify()` + `notificationImageHint` (correct freedesktop image-data), `commitDataRequest` quit handling, `bringForward`/`raiseWindow`.
- `trayicon.cpp` (pure image composition), `identicons.cpp` (fix hash seed), `notificationrules.cpp` (DND core, drop VIP/keywords if desired), `portalnotification.cpp` (fix NoBlock + add icon/sound), `webenginenotifproxy`.
- `performance.cpp`: flag fragment + start-up crash escalation (keep only disableGpu / ignoreBlocklist / webrtc shield & pipewire / cache type).
- `dictionaries.cpp` core path resolution + `syncDictionaryDirs` (BDic magic check) + CMake conversion block; `dictionarymanager.cpp` (verified download). Replace the delegate with a plain list.
- `passlock.cpp` (PBKDF2), `screenlock.cpp`, `autolockeventfilter.h`; rebuild `Lock` as a window-level overlay covering all windows.
- `utils.cpp`: `isSafeToDelete`, `processMemoryInfo`, `appDebugInfoMarkdown`, install-type detection; `debuglog.cpp` whole.
- `networkproxy.cpp`, `autostart.cpp` (fix `--hidden`, quoting, Flatpak/AppImage/snap launch commands, add macOS LaunchAgent), `updatechecker.cpp` (opt-in, fix failed-check stamping).
- `sessionbackup.cpp` (move copy off the GUI thread; hash-based change detection).
- `common.cpp` helpers, `Theme` palettes (make const).
- `privacyblur`, `customcss`, `webfont`, `chatwallpaper` (small, CSS-only, low risk) — optional.
- `shortcuts.cpp` registry + `commandpalette` (small, clean).
- `settingssearch.cpp` — only if the settings page stays large.
- SingleApplication (update to a tagged release; use JSON for all IPC payloads, never `split(" ")`).

**Tests / tooling / CI**
- The three-tier test layout; `QStandardPaths::setTestModeEnabled`, offscreen QPA, dedicated settings namespace; env base-URL overrides + loopback mock server; `TstScriptInstall` (install every script on a real profile); `TstScriptLiterals`; Flatpak-manifest policy test; language-label uniqueness test. Split the 6k-line file into one file per module.
- `tools/integration.sh` idea (drive the real binary via CLI/IPC under a throwaway HOME) and CDP screenshot scripts.
- CI: exact Qt pin with reason; `create-release` from CHANGELOG section; post-bundle verification (NSS modules, WebEngine runtime files, CRT DLLs, MSI size); snap edge/stable; Scorecard; Codecov. Pick **3 targets** for v1 (AppImage+zsync, Flatpak, Windows MSI with SignPath) and add deb/rpm/snap/AUR later.
- `dictionaries` fixed-tag release + SHA-256 manifest for on-demand assets.

### 9.2 Drop
- **All messaging automation & APIs**: scheduled messages, reminders, quick compose, `--send`/templates/`--var`, send-by-name, attachment sender, auto-reply, local HTTP API, Cloud Business API, webhooks (≈4.5k lines, ToS risk, page-reload sender, security holes).
- **AI assistant, Ollama, inline translation, chat export** (≈2k lines, privacy surface, DOM scraping).
- **Automatic sunrise/sunset theme + Sunclock + QtPositioning dependency** — follow-system-theme covers it.
- **RateApp, MoreApps, three donation buttons** (dead/upstream promo).
- **Grid view, tab tear-off/detached windows, window-layout persistence, absorb logic, nudge hack** (keep at most a simple in-window tab bar, or rely on `--profile`).
- **Chat-list strip**, **muted-status hide**, **focus mode**, **HD AB flag**, **linked-device name patch** (optional: keep only if you accept Meta-internal breakage), **custom JS addons**, **chat colour themes** (14) — or keep themes only if re-implemented as a static CSS override set.
- **Custom frameless title bar + resizer + tabs-in-title-bar.**
- **Spell-check focus/cycle + tray spelling submenu + hand-painted delegate**; bundle ~5 common dictionaries + download the rest via a plain list.
- **14 of the 18 performance knobs** (keep disable-gpu, ignore-blocklist, WebRTC shield, PipeWire, cache type).
- **Runtime accordion rebuild + settings-state-across-restart** — design a smaller settings page with 3–4 tabs instead; aim for < 40 options so search is unnecessary.
- **`checkLoadedCorrectly` storage-wipe**, `loadingQuirk` retry dance, `RequestInterceptor`, `injectNewChatJavaScript` (build the `/send?phone=` URL in C++ and `load()` it).
- **Certificate-error override dialog** (hard-fail with a message for a single-origin app), `LocalContentCanAccess*` attributes, HTTP-auth dialogs unless needed.
- **Custom `NotificationPopup` + `ScrollText` marquee** — use native notifications everywhere; fall back to `QSystemTrayIcon::showMessage`.
- **Inline-reply D-Bus backend** (or keep, but it requires the send-by-name automation you are dropping → drop).
- **Legacy migration code** (org rename, WhatSie→whatly, `--migrate-from`, `permissionPromptsFixed`, `normalizeWindowTheme` repair) — a new app has no legacy.
- **Profile export/import via external `tar`** — if wanted, use `QuaZip`/`libarchive` or a plain directory copy.
- **250 flag PNGs, store-category icons, `resize.sh` in qrc, `.dic_delta` files, `dictionaries/urls`, `tools/video/`, `make-cards`/`make-video`, legacy release workflows, `.pre-commit` conventional-commit hook, `docs/CHANGELOG.md` duplicate, Gentoo/OBS/Flathub stubs** until proofed.
- **X11 `XGrabKey` global shortcut** (keep portal path only) and Xlib Caps-Lock detection (use `QGuiApplication::queryKeyboardModifiers` or accept absence).

### 9.3 Design rules for the rewrite (derived from whatly's pain points)
1. A `MainWindow` under ~800 lines: window/tray/lifecycle only. Notifications, permissions, downloads, profile, lock, theme each in their own class with a narrow interface; no `qobject_cast<MainWindow*>(parent())` anywhere.
2. One `QSettings` schema header (`struct Key { const char* name; QVariant def; }`) consumed by both the settings UI and the modules; keys grouped `window/`, `web/`, `notif/`, `lock/`, `perf/`, `net/`. Secrets (if any) via `QtKeychain` or not at all.
3. All injected JS in `.js` resource files under `:/js/`, loaded through one helper that substitutes a JSON config object (`window.__app = {...}`); never string-concatenate user text into JS — pass through `QJsonDocument`. Keep every script idempotent and self-gated on a live flag; keep body-wide `MutationObserver`s out; prefer timers with budgets.
4. Whitelist of WhatsApp DOM anchors in one place (`#pane-side`, `#main`, `footer [contenteditable]`, `[data-icon=…]`), with a test that installs every script on a real profile and a CDP-based smoke test that can be run manually against a live session.
5. Nothing blocks the GUI thread: no nested `QEventLoop`, no synchronous D-Bus introspection, no `tar`, no recursive copies at startup.
6. Every "requires restart" setting is a smell; if unavoidable, keep the `--restart-wait` pattern but limit to Chromium flags.
7. Notifications: libnotify/portal on Linux (with `image-data`, `desktop-entry`, `sound-name`, `category`), `QSystemTrayIcon` elsewhere, click → raise + focus composer. That is all.
8. Multi-account = `--profile` first; in-window tabs only if they stay under ~400 lines.
9. Lock = app-level overlay over *every* window + block actions/IPC while locked; PBKDF2; attempt throttling.
10. Ship: AppImage (+zsync), Flatpak, Windows MSI (SignPath), snap; exact Qt pin; changelog-driven release notes; unit + script-install + manifest-policy tests in CI from day one.
