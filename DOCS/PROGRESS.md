# Progress log

Newest first. One entry per work session. Keep entries factual: what changed, what was verified,
what is blocked. Milestone status table at the bottom.

---

## 2026-08-27 — M0 Foundation

**Done**
- Surveyed `../whatsie` (≈6 k LOC) and `../whatly` (≈35 k LOC); reports in `reference/`.
- Pulled all GitHub issues/PRs for both repos (297 + 39 issues, 40 + 61 PRs) → `reference/github-issues-2026-08-27.md`.
- Wrote `CODING_STANDARDS.md`, `LESSONS.md`, `DECISIONS.md` (ADR-001…013), `FEATURES.md`
  (≈110 rows with recommendations, awaiting owner decisions), `ROADMAP.md`.
- Scaffold: top-level CMake (Qt 6.11, C++20), `cmake/Warnings.cmake`, `cmake/Version.cmake`,
  `cmake/SnapSdkWorkaround.cmake`, `.clang-format`, `.clang-tidy`, `.editorconfig`, `.gitignore`.
- Layered targets: `whatsie_core` (logging, `Settings` facade + keys), `whatsie_platform`
  (`describeHost`, `isSandboxed`), `whatsie_web` (`WebProfile`, `WebPage` console→log,
  `WebView`, `sanitizeUserAgent`), `whatsie_ui` (`MainWindow`), `whatsie_app` (`Application`,
  generated `version.h`), executable `whatsie`.
- Tests: `tst_settings` (7 cases), `tst_user_agent` (4), `tst_smoke` (offscreen MainWindow with
  WebEngine). All pass.
- Dev scripts: `scripts/dev-build.sh` (kde-qt6-core24-sdk snap, Qt 6.11.1),
  `scripts/dev-run.sh` (kf6-core24 runtime; `--ctest` mode), `scripts/snap-runtime-env.sh`.
  Test environment baked into CTest properties so plain `ctest` works from the host shell.

**Verified**
- `scripts/dev-build.sh --tests`: clean build, 0 warnings, 3/3 tests pass.
- `scripts/dev-run.sh` on Wayland/KDE: window opens, WhatsApp Web login page loads, page console
  visible under `whatsie.web.js`, UA reported as Chrome/140 (Qt 6.11.1 Chromium) without the
  QtWebEngine token.

**Gotchas recorded**
- `qt_add_executable` finalizer runs `qmlimportscanner`, which fails from the SDK snap on
  non-Ubuntu hosts → `QT_QML_MODULE_NO_IMPORT_SCAN TRUE` on every executable (no QML anyway).
- `ctest` must run under the *host* environment (it links host libssl); only the test binaries
  get the runtime-snap `LD_LIBRARY_PATH`.
- CMake warns about `libGLX/libOpenGL` runtime search path conflicts with the SDK — benign.
- Portal warning "Could not register app ID org.keshavbhatt.whatsie" is expected until a
  `.desktop` file is installed (M4).

**Blocked / waiting**
- Owner decisions in `FEATURES.md` (Decision column) and `DECISIONS.md` open questions OQ-1…4.
- Nothing else; M1 can start on CORE rows while decisions on NICE rows are pending.

---

## Milestone status

| Milestone | Status | Notes |
|---|---|---|
| M0 Foundation | ✅ done | 2026-08-27 |
| M1 Usable shell | — | waiting for green light |
| M2 Notifications | — | |
| M3 Web integration | — | |
| M4 Packaging & CI | — | |
| M5 Approved extras | — | needs `FEATURES.md` decisions |
| M6 Windows/macOS | — | needs OQ-3 |
