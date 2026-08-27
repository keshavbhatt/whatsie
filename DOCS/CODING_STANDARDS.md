# Coding Standards — new-whatsie (Qt 6 / C++20)

These are the rules the rewrite follows. They are binding for every commit. When a rule is
violated for a good reason, the reason goes in a code comment and (if architectural) in
`DOCS/DECISIONS.md`.

The document is deliberately opinionated. The original whatsie grew organically into a few
giant classes with UI, business logic, JavaScript strings and settings access all mixed
together; whatly fixed much of that but at the cost of size. The goal here is **small,
boring, testable code**.

---

## 1. Language & toolchain

| Item | Rule |
|---|---|
| Language | C++20 (`CMAKE_CXX_STANDARD 20`, `CMAKE_CXX_STANDARD_REQUIRED ON`, no extensions) |
| Qt | Qt 6.8 LTS minimum (see `DECISIONS.md` ADR-001). No Qt5 compatibility shims. |
| Build | CMake ≥ 3.21, `qt_standard_project_setup()`, `qt_add_executable`, `qt_add_resources`, `qt_add_translations`. No qmake. |
| Compiler | GCC ≥ 12 / Clang ≥ 15 / MSVC 2022. Warnings on: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wnon-virtual-dtor -Wold-style-cast`. Warnings are errors in CI (`-Werror` behind `WHATSIE_WERROR` option). |
| Formatting | `clang-format` with the repo `.clang-format`. Formatting is not reviewed by humans; CI runs `clang-format --dry-run --Werror`. |
| Static analysis | `clang-tidy` with the repo `.clang-tidy` (readability, modernize, bugprone, performance, cppcoreguidelines subset). Runs in CI on changed files. |
| Tests | Qt Test (`QTest`) via `qt_add_test`/CTest. Every non-UI module ships a test. |

---

## 2. Project layout

```
new-whatsie/
├── CMakeLists.txt              # top-level; options, Qt find, subdirs
├── cmake/                      # helper modules (Version.cmake, Packaging.cmake, Warnings.cmake)
├── src/
│   ├── main.cpp                # tiny: create Application, run
│   ├── app/                    # Application (QApplication subclass), single-instance, CLI parsing
│   ├── core/                   # Pure logic, NO QtWidgets, NO QtWebEngine. Unit-tested.
│   │   ├── settings/           # Settings facade (typed accessors over QSettings)
│   │   ├── theme/
│   │   ├── lock/
│   │   └── ...
│   ├── web/                    # WebEngine layer: profile, page, view, scripts, interceptor
│   │   └── scripts/            # *.js injected into WhatsApp Web (resource files, never string literals)
│   ├── platform/               # OS-specific: autostart, notifications backends, tray quirks
│   │   ├── linux/
│   │   ├── windows/
│   │   └── macos/
│   ├── ui/                     # QtWidgets: MainWindow, dialogs, settings pages, tray
│   │   └── widgets/            # reusable small widgets
│   └── resources/              # icons.qrc, images, sounds
├── tests/                      # one tst_<module>.cpp per core module + a smoke test
├── packaging/                  # snap/, flatpak/, debian/, appimage/, windows/
├── translations/               # *.ts (optional, later milestone)
└── DOCS/                       # everything about the project, see DOCS/README.md
```

Dependency direction is strict and one-way:

```
ui  ──►  web  ──►  core
 │        │
 └──► platform ─► core
```

* `core/` must compile with only `Qt6::Core` (+ `Qt6::Gui` for `QColor`/`QIcon` if unavoidable).
* `web/` may use `Qt6::WebEngineWidgets` and `core/`.
* `ui/` may use everything. Nothing depends on `ui/`.
* `platform/` provides interfaces in `core/` (e.g. `core::AutostartBackend`) and implementations
  under `platform/<os>/`, selected in CMake, not with `#ifdef` soup inside classes.

Every directory is its own `add_library(whatsie_<name> STATIC)` target with explicit
`target_link_libraries`. This makes the dependency arrows enforceable by the linker.

---

## 3. Naming

| Thing | Convention | Example |
|---|---|---|
| Namespace | `whatsie::<layer>` | `whatsie::core`, `whatsie::ui` |
| Class / struct / enum | `PascalCase` | `LockScreen`, `NotificationBridge` |
| Method / free function | `camelCase` | `setZoomFactor()` |
| Member variable | `m_camelCase` | `m_webView` |
| Static member | `s_camelCase` | `s_instance` (should be rare) |
| Constant / constexpr | `kPascalCase` | `constexpr int kDefaultZoomPercent = 100;` |
| Enum values | `PascalCase` in `enum class` | `Theme::Dark` |
| Signals | past tense or noun-ish, no `on` prefix | `unreadCountChanged(int)` |
| Slots / handlers | `on<Source><Event>` only when connected from `.ui` auto-connect (we don't use auto-connect, so: verb phrase) | `applyTheme()` |
| Files | `snake_case.cpp/.h` matching the primary class: `lock_screen.h` → `class LockScreen` | |
| Settings keys | `section/camelCase` | `"window/zoomFactor"` |
| Qt properties | `camelCase` | |

One class per file pair. Header guards: `#pragma once`.

---

## 4. Classes & ownership

1. **Qt parent-child ownership** for every `QObject`. `new Foo(this)` is fine; `new Foo()` with a
   later `setParent` is not. A `QObject` without a parent must be held in `std::unique_ptr`
   or `QScopedPointer`, never a bare pointer.
2. Non-`QObject` types: value semantics or `std::unique_ptr`. `std::shared_ptr` requires a
   justification comment.
3. Raw pointers are **non-owning observers only**. Never `delete` a raw pointer outside a
   destructor of the owner.
4. Rule of zero. If you write a destructor, you must explain why.
5. No singletons except: `Application` (via `qApp` cast helper) and `Settings` (one instance
   owned by `Application`, passed by reference to whoever needs it). Nothing else may be
   reached through a global. **Pass dependencies through constructors.**
6. `Q_OBJECT` on every `QObject` subclass. `Q_DISABLE_COPY_MOVE` on every `QObject` subclass.
7. Prefer composition over inheritance. Do not subclass `QMainWindow` to hold app logic — the
   window owns controllers, it isn't one.
8. Hard size limits: a class > 500 lines or a function > 60 lines is a review blocker. Split it.
9. No `friend`. No `protected` data members.
10. Interfaces (`class INotifier { public: virtual ~INotifier() = default; ... }`) for anything
    with more than one implementation (platform backends, test doubles).

---

## 5. Qt idioms

* **Signals/slots**: always the pointer-to-member-function syntax. `SIGNAL()`/`SLOT()` string
  macros are forbidden. Lambdas must capture `this` only when a context object is given:
  `connect(obj, &Obj::sig, this, [this]{ … });` — never a context-less lambda on a long-lived
  sender.
* **No `.ui` files.** Build widgets in code. Reasons: diffable, testable, no `ui_*.h` coupling,
  no generated-code mystery. Layouts belong in a private `setupUi()` method that is only layout,
  no logic.
* **Strings**: `QStringLiteral("…")` for literals, `u"…"_s` (`using namespace Qt::StringLiterals`)
  is acceptable and preferred in new code. `tr()` for anything the user sees. No `QString::fromLatin1`
  for constants. Never build JavaScript by string concatenation — see §7.
* **Containers**: `QList`/`QHash` when the value crosses a Qt API boundary; `std::` otherwise.
  Iterate with range-for; `qAsConst`/`std::as_const` when iterating a Qt container that would detach.
* **Enums** used in settings or signals are `enum class` registered with `Q_ENUM`/`Q_ENUM_NS`.
* **Timers**: `QTimer::singleShot` with a context object. No busy waiting, no `processEvents()`.
* **Threads**: none unless measured. If needed: `QThread` + worker object or `QtConcurrent::run`
  with `QFutureWatcher`. Never touch widgets off the GUI thread.
* **Logging**: `Q_LOGGING_CATEGORY` per module (`whatsie.web`, `whatsie.tray`, …). `qDebug()`
  without a category is forbidden. No `std::cout`/`printf`.
* **Deprecated API** is a build error (`QT_DISABLE_DEPRECATED_UP_TO=0x060800`).
* `QT_NO_CAST_FROM_ASCII`, `QT_NO_CAST_TO_ASCII`, `QT_NO_KEYWORDS` are defined globally
  (use `Q_SIGNALS`/`Q_SLOTS`/`Q_EMIT`).
* Use `QStandardPaths` for every path. Never hardcode `~/.config`, `/tmp`, `AppData`.
* Use `QSettings` **only** through the `core::Settings` facade (typed getters/setters with
  defaults in one place). Direct `QSettings` construction outside `core/settings/` is forbidden.

---

## 6. Error handling

* No exceptions across Qt boundaries. Internal code may use exceptions for programmer
  errors only; never for control flow. Qt is compiled without exception guarantees anyway.
* Fallible operations return `std::optional<T>` or a small `Result<T>` (`std::expected` once
  the minimum compiler allows) — never a bool plus out-parameter.
* Every failure that the user could care about is logged with a category and surfaced through
  a signal (`errorOccurred(QString userMessage)`), never a `QMessageBox` from inside `core/`
  or `web/`. Only `ui/` shows dialogs.
* `Q_ASSERT` for invariants, never for input validation.

---

## 7. WebEngine & injected JavaScript

WhatsApp Web changes without notice. The rewrite treats every injected script as a
**contract that can break**, so:

1. Scripts live in `src/web/scripts/*.js`, are loaded with `qt_add_resources`, and injected via
   `QWebEngineScript` with explicit `injectionPoint`, `worldId` and `runsOnSubFrames`, on the
   **profile-level** collection (`profile->scripts()`), registered before the first load.
   Page-level collections are not used: on Qt 6.11, changing `page->scripts()` right before a
   navigation drops `DocumentCreation` injection for that navigation (ADR-006).
   No JavaScript inside C++ string literals. No `runJavaScript(QString::arg(...))` templating —
   pass data through `QWebChannel` or through a single `window.__whatsie` config object
   injected once. User-provided text (phone numbers, deep-link text) never reaches JS by
   concatenation; build URLs in C++ or serialise through `QJsonDocument`.
1. Scripts may rely only on the *stable tier*: standard web APIs, our own `<style>` ids, and
   structural anchors (`#pane-side`, `#main`, `#side`, `footer [contenteditable]`). Anything that
   needs `window.require('WAWeb…')`, React fiber internals, IndexedDB schema or localised text is
   rejected in review.
2. Each script has a header comment: purpose, which DOM/API it depends on, the date it was last
   verified working, and what happens when it fails (must be *no-op*, never break the page).
3. Every script is wrapped in an IIFE with `try/catch` that reports to the C++ side through
   `QWebChannel` (`bridge.scriptFailed(name, message)`) so failures are visible in logs.
4. Communication from JS to C++ goes through **one** `QWebChannel` object (`web::Bridge`) with
   a small, documented API. No `console.log` sniffing, no title polling, no URL-scheme hacks.
5. User-agent is a single constant in `web/user_agent.h`, overridable from settings,
   with the reasoning documented (why Chrome, why that version).
6. The `QWebEngineProfile` is persistent, named, and its storage path comes from
   `QStandardPaths`. Cache/cookies/permissions are set up in exactly one function.
7. Permissions (`QWebEnginePermission`, Qt ≥ 6.8) are handled in `web/`, decisions are stored
   through the settings facade, and the **UI decides** how to ask (signal out, slot in).
8. `QWebEnginePage::javaScriptConsoleMessage` is routed to the `whatsie.web.js` logging category.

---

## 8. Settings

* One `core::Settings` class, typed accessors, defaults in one table:
  ```cpp
  [[nodiscard]] Theme theme() const;
  void setTheme(Theme);
  Q_SIGNAL void themeChanged(Theme);
  ```
* Keys are `constexpr` in `settings_keys.h`; never spelled twice.
* Settings emit a signal on change; UI and web layers *react* to the signal. Nobody polls.
* No migration code from old whatsie — see project brief; a fresh profile is acceptable.

---

## 9. Testing

* `core/` is pure logic and must have ≥ 80 % line coverage (checked in CI, not enforced as a
  gate for now).
* `web/` gets tests for the script bundle (files exist, parse as JS via a tiny `QJSEngine`
  syntax check) and for the user-agent/permission logic.
* `ui/` gets a smoke test: construct `MainWindow` offscreen (`QT_QPA_PLATFORM=offscreen`),
  show, process events, destroy — no crash, no leak warnings.
* Tests use only public API. If something is untestable through public API, the design is wrong.
* Test file naming: `tests/tst_<module>.cpp`, one `QObject` class per file, `QTEST_MAIN` or
  `QTEST_GUILESS_MAIN`.

---

## 10. Git & review

* Conventional commits: `feat:`, `fix:`, `refactor:`, `docs:`, `build:`, `test:`, `chore:`.
  Scope in parentheses is the layer/module: `feat(tray): …`.
* One logical change per commit. A commit must build and pass tests on its own.
* Every feature PR updates `DOCS/PROGRESS.md` and, if it adds a user-visible option,
  `DOCS/FEATURES.md`.
* No commented-out code. No `TODO` without an issue number: `// TODO(#42): …`.
* No dead code "for later". Git remembers.

---

## 11. Things explicitly banned (lessons from whatsie / whatly)

| Banned | Why | Do instead |
|---|---|---|
| `MainWindow` doing WebEngine setup, tray, lock, notifications, downloads | god-class, untestable | separate controllers owned by the window |
| Multiple `.cpp` files for one class (`mainwindow_tray.cpp`, …) | hides that the class is too big | split the class |
| JS in C++ string literals / `QString::arg` templating | unreadable, injection-fragile | resource `.js` files + `QWebChannel` |
| Direct `QSettings` everywhere with retyped keys | typos, no defaults, no signals | `core::Settings` |
| `#ifdef Q_OS_*` blocks inside business logic | unreadable, untestable | platform backends behind interfaces, chosen in CMake |
| `QMessageBox` / dialogs from non-UI code | couples logic to widgets | signals + UI handles them |
| `SIGNAL()`/`SLOT()` macros, `.ui` files, auto-connect slots | not type-checked, hidden coupling | PMF connects, code-built widgets |
| Global mutable state (`static QString g_…`), `extern` variables | untraceable | constructor injection |
| Third-party code copied into `src/` (vendored `singleapplication`, `libnotify-qt`) without a manifest | unmaintained forks | `FetchContent`/submodule under `third_party/` with version pinned + `THIRD_PARTY.md` |
| Feature flags via magic settings keys | undiscoverable | explicit option in settings facade + UI |
| Blocking network / disk on GUI thread (`QEventLoop` spin) | freezes | async signals |
| Hardcoded user-agent strings spread around | breaks with WhatsApp changes | one constant, one override |
| Ignoring WebEngine console messages / script errors | silent breakage | route to logging categories |

---

## 12. Definition of Done (per feature)

- [ ] Code follows every section above; `clang-format` & `clang-tidy` clean.
- [ ] Unit tests for all `core/` logic touched; smoke test still passes.
- [ ] Works on Linux (X11 + Wayland); Windows/macOS-specific code behind platform backends.
- [ ] Logging category added/used; no silent failures.
- [ ] `DOCS/FEATURES.md` row updated (status, implementing class).
- [ ] `DOCS/PROGRESS.md` entry.
- [ ] No new third-party dependency without an ADR in `DOCS/DECISIONS.md`.
