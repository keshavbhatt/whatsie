# Packaging

## Snap (M4 — done)

`snap/snapcraft.yaml` builds Whatsie against the **kde-neon-6** extension, which provides
**Qt 6.11** from the `kf6-core24` content snap — the same runtime `scripts/dev-run.sh` targets.
The extension also provides the GPU (mesa-2404), audio (PulseAudio/PipeWire) and portal wiring
that dev-run has to add by hand, so the shipped snap needs none of those workarounds.

- **App id:** `com.ktechpit.whatsie` (ADR-014). Desktop file, AppStream metainfo and hicolor
  icons are installed by CMake (`dist/linux/`, top-level `install()` rules); `adopt-info` reads
  the version from the metainfo.
- **Sandbox** (ADR-008): to avoid a store reviewer
  declaration, the snap uses plain `browser-support` and passes `--no-sandbox` via the app
  `environment` — isolation comes from strict snap confinement. Native and Flatpak builds keep
  the Chromium sandbox (the app never hard-codes `--no-sandbox`).

### Building — in CI, not locally

We do **not** build the snap on developer machines (it pulls a multi-GB KDE SDK into an LXD
container). CI does it: `.github/workflows/snap.yml` runs `snapcore/action-build` on every push
to `main`, uploads the `.snap` as an artifact, and — when the repo variable
`PUBLISH_TO_STORE=true` and the `SNAPCRAFT_STORE_CREDENTIALS` secret are set — releases it to the
**edge** channel.

To build locally anyway (not recommended): `snapcraft --use-lxd`. Clean up afterwards with
`snapcraft clean` and `lxc --project snapcraft list`/`delete` — the build containers are large.

## Windows (x64)

Built in CI on `windows-2022` with MSVC — see `.github/workflows/windows.yml`. We do
not build Windows on developer machines.

- **Qt:** `jurplel/install-qt-action` installs Qt 6.11 (`win64_msvc2022_64`) with the
  `qtwebengine qtwebchannel qtpositioning` modules — the same Qt major the app requires
  everywhere.
- **Build:** `cmake -G "Visual Studio 17 2022" -A x64` → `cmake --build --config Release`.
  The app icon and version info come from `dist/windows/whatsie.rc.in` (embedded via the
  `WIN32` block in `src/CMakeLists.txt`; the icon is `dist/windows/whatsie.ico`).
- **Deploy:** `windeployqt` stages the Qt/Chromium runtime; the MSVC CRT DLLs are then
  copied app-local (a plain `windeployqt` leaves only `vc_redist.exe`, which a portable
  zip can't run) and a step fails the build if any required DLL is missing.
- **Packaging:** a portable **zip** and a **WiX MSI** (`packaging/windows/whatsie.wxs` —
  per-machine, Start Menu + Desktop shortcuts, in-place upgrades via a fixed
  `UpgradeCode`; the deployed tree is auto-harvested). Both are uploaded as CI artifacts
  on every push; a version tag (`v6.0.0`) also attaches them to that GitHub Release.
- **Autostart:** a per-user registry `Run` entry (`src/platform/windows/`), the Windows
  counterpart to the Linux XDG autostart file.

Known limitation: the official QtWebEngine binaries omit the proprietary **H.264/AAC**
codecs, so this build **cannot send MP4 videos** (WhatsApp rejects them as unsupported).
Photos, WebM/VP8/VP9 videos, messaging and screen share all work, and voice calls (Opus)
are unaffected. Qt links FFmpeg statically — there is no drop-in codec pack — so enabling
H.264 needs a separate multi-hour QtWebEngine rebuild, a possible future addition. Code
signing (to avoid the SmartScreen "unknown publisher" prompt) is likewise a later step;
the workflow can gain a secret-gated signing stage without other changes.

## Flatpak (later)

Planned on `org.kde.Platform` 6.x with portals for files/notifications. Not started yet.

## Other targets

AppImage / deb / rpm / AUR / macOS are not release targets (ADR-016 keeps v1 to snap + Windows).
