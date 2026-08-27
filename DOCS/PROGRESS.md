# Progress log

Newest first. One entry per work session. Keep entries factual: what changed, what was verified,
what is blocked. Milestone status table at the bottom.

---

## 2026-08-27 — Scope decisions (owner review)

**Done**
- Owner reviewed every `FEATURES.md` row through an interactive Q&A (4 rounds). All decisions
  recorded in the Decision column; summary in "Approved scope at a glance".
- Open questions resolved as ADR-014 (app id `com.ktechpit.whatsie`), ADR-015 (app lock kept,
  hardened, M5), ADR-016 (Linux + Windows), ADR-017 (spell check: system language + on-demand).
- Notable calls: all CORE rows kept; all DROP rows confirmed; kept extras S11, S13, S14, S16,
  S20, S26, T3, T5, T6, N3, N5, N7, N10, A7, A8, A14 (setting, default off), M6, P2, P3, P4, D4;
  dropped S24 (configurable shortcuts), S27 (translations), A9 (custom CSS), M11, P11;
  LATER: S23, T8, P8, X2.
- Skeleton updated: identity constants now `ktechpit` / `com.ktechpit.whatsie`;
  `ScrollAnimatorEnabled` default off. Rebuilt, tests green.

**Next:** M1 (usable shell) can start — no blockers.

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
- Portal warning "Could not register app ID …" is expected until a `.desktop` file is
  installed (M4).

**Blocked / waiting**
- Owner decisions in `FEATURES.md` — resolved the same day (see entry above).

---

## Milestone status

| Milestone | Status | Notes |
|---|---|---|
| M0 Foundation | ✅ done | 2026-08-27 |
| M1 Usable shell | — | green light given 2026-08-27 |
| M2 Notifications | — | |
| M3 Web integration | — | |
| M4 Packaging & CI | — | |
| M5 Approved extras | — | list fixed in `ROADMAP.md` |
| M6 Windows | — | ADR-016 |
