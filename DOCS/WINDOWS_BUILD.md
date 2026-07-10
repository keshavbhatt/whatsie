# Building Whatsie on Windows

Whatsie builds natively on Windows 10 and later. The same codebase is used on
all platforms; Linux-only pieces (libnotify-qt/D-Bus notifications, X11) are
compiled out via `Q_OS_*` guards and replaced with Qt/Win32 equivalents.

## Requirements

| Component | Notes |
|---|---|
| Visual Studio 2022 (Community is fine) | "Desktop development with C++" workload. **MSVC is required** — Qt WebEngine does not support MinGW. |
| Qt 6.10+ for MSVC 64-bit | Via the Qt Online Installer. Explicitly select the **Qt WebEngine** and **Qt Positioning** additional libraries — WebEngine is unchecked by default. |
| CMake 3.24+ | Standalone, or the one bundled with Visual Studio. |
| Git | Used by the build to embed commit info. |

The `src/libnotify-qt` submodule is **not** needed on Windows; you can clone
without `--recurse-submodules`.

## Build

From a *x64 Native Tools Command Prompt for VS 2022* (or any shell where
CMake can find MSVC):

```bat
git clone https://github.com/keshavbhatt/whatsie.git
cd whatsie

cmake -G "Visual Studio 17 2022" -A x64 -B build ^
      -DCMAKE_PREFIX_PATH=C:\Qt\6.10.0\msvc2022_64
cmake --build build --config Release
```

The executable lands in `build\Release\whatsie.exe`.

Alternatively, open the project folder in Qt Creator and build with an
MSVC-based Qt 6.10+ kit — no extra configuration needed.

## Running / deploying

To run outside the build environment, place the Qt runtime next to the
executable with `windeployqt` (ships with Qt):

```bat
C:\Qt\6.10.0\msvc2022_64\bin\windeployqt.exe build\Release\whatsie.exe
```

This copies the required Qt DLLs, the WebEngine process
(`QtWebEngineProcess.exe`) and its resources.

## Platform behavior notes

* **Notifications** — the "native" notification option uses
  `QSystemTrayIcon::showMessage`, which Windows 10+ renders as toast
  notifications. The in-app popup option works unchanged.
* **Caps Lock detection** on the lock screen uses `GetKeyState(VK_CAPITAL)`
  instead of X11.
* **Downloads / "Open folder"** use `QDesktopServices` instead of `xdg-open`.
* The default user agent reports `Windows NT 10.0` so WhatsApp Web treats the
  app as Chrome on Windows.
* The executable is a GUI-subsystem binary (no console window); icon and
  version info come from `dist/windows/whatsie.rc.in`.
