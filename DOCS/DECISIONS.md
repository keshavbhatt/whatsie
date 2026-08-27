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

## ADR-007 — User agent derived from the engine default (2026-08-27)

**Context.** Lesson A7.

**Decision.** `web::sanitizeUserAgent()` strips the `QtWebEngine/x.y.z` token from
`QWebEngineProfile::httpUserAgent()`; the real Chromium version stays. A settings override exists
for emergencies. No per-OS hard-coded UA strings.

**Consequences.** Upgrading Qt upgrades the advertised browser automatically.

## ADR-008 — Chromium sandbox stays on (2026-08-27)

**Context.** Lesson A8.

**Decision.** Never pass `--no-sandbox`. Snap: `browser-support` with `allow-sandbox: true`.
Flatpak: default. User can still override via `QTWEBENGINE_CHROMIUM_FLAGS`.

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

---

## Open questions (need an owner decision)

- **OQ-1** Application id / org name: skeleton uses `org.keshavbhatt.whatsie`
  (desktop file id, settings path `~/.config/org.keshavbhatt/whatsie.conf`). whatsie used
  `com.ktechpit.whatsie` for the desktop id but `org.keshavnrj.ubuntu` for settings. Pick one;
  the snap/flatpak ids must match it.
- **OQ-2** App lock: keep (with the hardened design in A10) or drop for v1?
- **OQ-3** Windows/macOS: in scope for v1 or Linux-first with platform backends stubbed?
- **OQ-4** Spell check: ship system-language `.bdic` only, on-demand others, or drop?
