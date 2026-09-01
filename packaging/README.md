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

## Flatpak (later)

Planned on `org.kde.Platform` 6.x with portals for files/notifications. Not started yet.

## Other targets

AppImage / deb / rpm / AUR / macOS are not release targets (ADR-016 keeps v1 to snap + Windows).
