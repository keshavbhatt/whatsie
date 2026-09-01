# Whatsie

A lightweight WhatsApp Web desktop client built with Qt 6 WebEngine — WhatsApp in its
own window with native desktop notifications, a system-tray unread badge, light/dark
themes that follow the page, zoom and interface scaling, a real downloads manager, voice,
video and screen-share calls, spell check, a network proxy, and an optional app lock.

## Features

- Native notifications with avatars; system-tray icon with unread badge, mute and
  do-not-disturb.
- Light / dark / follow-system theming, kept in step with WhatsApp Web.
- Voice, video and screen-share calls (Wayland via the desktop portal).
- Downloads window with history; drag-and-drop and paste of attachments.
- Per-site camera/microphone/location controls; privacy blur; passcode app lock.
- System-language spell check; network proxy (HTTP / SOCKS5) with authentication.
- Single instance with `--profile` for multiple accounts; autostart at login.

## Build

Requires Qt **6.11** (the version shipped by the snap `kf6-core24` runtime and the Flathub
KDE runtime), CMake ≥ 3.21, and a C++20 compiler.

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
```

## License

MIT.
