# Whatsie (rewrite)

A from-scratch rewrite of [whatsie](https://github.com/keshavbhatt/whatsie), the WhatsApp Web
desktop client built with Qt 6 WebEngine.

Why a rewrite: the original grew into a few thousand-line classes with WhatsApp-internals
hacks that broke on every WhatsApp Web change; the [whatly](https://github.com/shakaran/whatly)
fork fixed the engineering but added ~25 k lines of features most people never use. This
project keeps the good parts of both and stays small. The reasoning is in
[`DOCS/LESSONS.md`](DOCS/LESSONS.md); the scope in [`DOCS/FEATURES.md`](DOCS/FEATURES.md).

## Status

Milestone **M0 — Foundation** is done: layered CMake scaffold, settings facade, WebEngine
profile/page/view, tests, dev scripts, and the full documentation set. Scope has been decided
(Linux + Windows, ~24 settings, no automation/AI features). Next: M1, the usable shell. See
[`DOCS/PROGRESS.md`](DOCS/PROGRESS.md) and [`DOCS/ROADMAP.md`](DOCS/ROADMAP.md).

## Build

Requires Qt **6.11** (the version shipped by the snap `kf6-core24` runtime and the Flathub KDE
runtime), CMake ≥ 3.21, a C++20 compiler.

```sh
# Development build against the KDE Qt 6.11 snap SDK (any distro)
sudo snap install kde-qt6-core24-sdk kf6-core24
scripts/dev-build.sh --tests
scripts/dev-run.sh

# Or with a system Qt >= 6.11
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Layout

```
src/core      pure logic (settings, services) — Qt Core only, unit-tested
src/web       WebEngine profile/page/view, injected scripts, bridge
src/platform  OS backends behind interfaces (notifications, autostart, …)
src/ui        QtWidgets: main window, dialogs, tray
src/app       Application object, CLI, single instance
tests/        Qt Test suites (unit + offscreen smoke)
packaging/    snap, flatpak, …
DOCS/         roadmap, decisions, feature contract, progress, references
```

Coding rules: [`DOCS/CODING_STANDARDS.md`](DOCS/CODING_STANDARDS.md).

## License

MIT (same as whatsie).
