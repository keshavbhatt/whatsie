# Feature contract: whatsie vs whatly vs new-whatsie

**Purpose.** This is the contract for scope (ADR-013). Decisions were taken by the owner on
2026-08-27 (recorded via interactive review; see `PROGRESS.md`). `Status` is updated by the
implementer (`—` / `wip` / `done` / `dropped`).

Sources: `reference/analysis-whatsie.md` (§3), `reference/analysis-whatly.md` (§3, §9),
`reference/github-issues-2026-08-27.md` (themes, most-requested). **W#** = whatsie issue,
**Y#** = whatly issue.

**Recommendation legend** (what the analysis proposed)
- **CORE** — not usable/credible without it. **NICE** — cheap and requested. **DROP** — niche,
  high-maintenance, ToS/privacy risk, or depends on WhatsApp internals.

**Decision legend** (what the owner decided)
- **KEEP** — build it (milestone per `ROADMAP.md`). **LATER** — planned after v1. **DROP** — never.

Sizes are the LOC whatly spent on the feature (a proxy for maintenance cost).

---

## 1. Session, window, lifecycle

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| S1 | Persistent WhatsApp Web session (named profile, disk cache/cookies/permissions) | yes (singleton destroyed after QApplication) | yes, per-account | CORE | The product. Profile owned by the view (ADR-003). | **KEEP** | done |
| S2 | Window geometry/state persistence (Wayland-safe, screen-aware) | yes, but re-centres on cursor screen every start | yes, tracks normal geometry by hand | CORE | W#103, 135 | **KEEP** | done |
| S3 | Close button → minimize to tray *or* quit (setting) | yes | yes | CORE | | **KEEP** | done |
| S4 | Start minimized (setting + `--minimized`) | yes | yes | CORE | | **KEEP** | done |
| S5 | Never hide window when no tray is available | no (W#292) | yes | CORE | Lesson B7 | **KEEP** | done |
| S6 | Honour session logout/shutdown (`commitDataRequest`) | no (W#66) | yes | CORE | | **KEEP** | done |
| S7 | Fullscreen F11 + page-requested fullscreen with reliable exit | yes, flickers; exit broken (W#169, 333) | yes, flickers | CORE | Must honour `fullScreenRequested(false)` | **KEEP** | done |
| S8 | Single instance + IPC (raise, open URL, settings, lock, …) | vendored SingleApplication | same, lossy `split(" ")` | CORE | ADR-010: `QLocalServer` + JSON, profile-aware key | **KEEP** | done |
| S9 | CLI options (`--minimized`, `--profile`, `--log-file`, `--new-chat`, …) | 9 options | ~20 incl. automation | CORE (small set) | Only what maps to a UI action. | **KEEP** | done |
| S10 | `whatsapp://send?phone=` scheme handler + desktop actions | yes (`?`→`&` hack) | yes | CORE | Build `/send?phone=` URL in C++ and `load()` it — no JS. | **KEEP** | done |
| S11 | `https://wa.me/`, `api.whatsapp.com`, `chat.whatsapp.com` invite links | no (W#3, 172, 186) | yes | NICE | Cheap once S10 exists. | **KEEP** | done (parse; handler registration in M4) |
| S12 | Render-process crash recovery with back-off | dialog per crash | loop breaker | CORE | W#102, Y#28 | **KEEP** | done |
| S13 | Connection watchdog (WebSocket monkey-patch, reload after resume) | no (W#208) | yes, 2 strikes / 3 reloads | NICE | Policy pure and tested; reload only, never wipe. | **KEEP** | — |
| S14 | Auto-reload when network returns (`QNetworkInformation`) | no | no | NICE | Complements S13. | **KEEP** | — |
| S15 | "Loading quirk" / "Error" title → wipe storage + quit | yes | yes | DROP | ADR-011. Diagnostic page with explicit buttons instead. | **DROP** | dropped |
| S16 | Corrupt service-worker cache recovery (once per page) | no | yes (Y#43) | NICE | 25 lines, non-destructive. | **KEEP** | — |
| S17 | Session backup/restore of IndexedDB at startup | no | yes, synchronous | DROP | Blocks GUI; unencrypted session copy. | **DROP** | dropped |
| S18 | Low-disk warning + movable data dir | no | yes | DROP | | **DROP** | dropped |
| S19 | Restart-in-place (`--restart-wait`) | no | yes | DROP | Aim for ≤ 2 restart-required settings. | **DROP** | dropped |
| S20 | Wayland RHI failure → one-shot XCB relaunch; start-up crash escalation | no | yes (Y#3, 84) | NICE | Keep ~60 lines, off the happy path. | **KEEP** | — |
| S21 | Setup wizard | no | yes | DROP | | **DROP** | dropped |
| S22 | Custom frameless title bar + resizer | no | yes (400 LOC) | DROP | | **DROP** | dropped |
| S23 | Global hotkey to raise window (portal only) | no | yes (330 LOC) | LATER | W#10, 14. | **LATER** | — |
| S24 | Configurable local shortcuts with conflict detection | no | yes | NICE | | **DROP** | dropped |
| S25 | Command palette (Ctrl+K) | no | yes | DROP | | **DROP** | dropped |
| S26 | Shortcuts cheat-sheet (Help → Shortcuts) | text popup | yes | NICE | W#319. Generated from `QAction` list. | **KEEP** | — |
| S27 | Interface translations (`.ts`) + language picker | never loaded | 22 languages, MT | NICE | | **DROP** | dropped (English only) |

## 2. Tray

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| T1 | Tray icon with context menu | yes | yes | CORE | | **KEEP** | done |
| T2 | Unread badge on tray + window icon | title parsing, 10 PNGs | composed image, tested | CORE | Compose in code; count from bridge with title fallback. | **KEEP** | done (title-based; bridge count in M3) |
| T3 | Monochrome / symbolic tray icon option | no (W#93, 128) | yes (Y#14) | NICE | SVG symbolic variant. | **KEEP** | — |
| T4 | Left-click toggles window (setting) | yes | yes + Windows focus grace | CORE | W#274 | **KEEP** | done |
| T5 | Hide tray icon entirely (setting) | no (W#127, 193) | no | NICE | Requires S5. | **KEEP** | — |
| T6 | Connection-state dimmed icon | no (W#108) | yes | NICE | Needs S13. | **KEEP** | — |
| T7 | Recent-unread submenu / open chat by name | no | yes (IndexedDB) | DROP | WA private schema. | **DROP** | dropped |
| T8 | Launcher/dock unread badge (Unity LauncherEntry, Windows taskbar) | no (W#122, 165) | yes | LATER | Cheap once T2 exists. | **LATER** | — |

## 3. Notifications

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| N1 | Native Linux notifications via `org.freedesktop.Notifications` (id-tracked, avatar, identicon, click → raise) | libnotify-qt, id-less (W#5, 42, 271, 278) | libnotify-qt | CORE | ADR-009. Fix BGRA/RGBA (W#312, 328). | **KEEP** | done |
| N2 | Windows notifications via `QSystemTrayIcon::showMessage` | yes | yes | CORE | Windows in scope (ADR-016). | **KEEP** | done (tray-balloon backend; used as fallback on Linux too) |
| N3 | XDG portal notifications (Flatpak) | no | yes, cannot detect failure | NICE | Fix the `NoBlock` issue. | **KEEP** | done (untested outside Flatpak) |
| N4 | In-app custom popup widget | yes (default, leaks) | yes | DROP | Native everywhere; crash source (W#207/210/212). | **DROP** | dropped |
| N5 | Notification sound hint | no | yes | NICE | | **KEEP** | done |
| N6 | Disable notifications toggle + timeout | yes | yes | CORE | | **KEEP** | done |
| N7 | Do-Not-Disturb, manual (1h / 2h / until off) in tray | no | yes + schedule | NICE | Manual only; no schedule. | **KEEP** | done |
| N8 | Keyword highlights / VIP / muted contacts | no | yes | DROP | | **DROP** | dropped |
| N9 | Inline reply from notification | no | yes (DOM automation) | DROP | | **DROP** | dropped |
| N10 | Click notification → open that chat | raise only | JS focus | NICE | Via WA's own `Notification.onclick`; no DOM automation. | **KEEP** | done |
| N11 | Auto-grant web Notification permission | prompt (W#307) | auto | CORE | | **KEEP** | done |
| N12 | "Started minimized" informational popup | every launch | yes | DROP | Lesson B6. | **DROP** | dropped |
| N13 | Reply reminders | no | yes | DROP | | **DROP** | dropped |

## 4. Appearance & zoom

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| A1 | Theme: System / Light / Dark, persisted | light/dark, reverts (fixed 4×) | + follow system | CORE | Lesson B9. Page theme via WA `localStorage.theme` before load. | **KEEP** | done (Qt side; page theme in M3) |
| A2 | Live follow-system-theme | no (W#157) | portal | CORE | `QStyleHints::colorSchemeChanged`; portal fallback. | **KEEP** | done |
| A3 | Push theme into running page (React fiber, WA modules) | yes | yes | DROP | Reload on theme change instead. | **DROP** | dropped |
| A4 | Automatic sunrise/sunset theme | yes (bugs) | yes (680 LOC) | DROP | | **DROP** | dropped |
| A5 | Widget style selector | yes | yes | DROP | Fusion + palette. | **DROP** | dropped |
| A6 | Zoom: normal vs maximized, Ctrl+/-/0, Ctrl+wheel blocked, min-size scales | yes (W#192) | yes | CORE | Most-requested #3. | **KEEP** | done |
| A7 | Interface scale / font size setting | no (W#276) | yes | NICE | `QT_SCALE_FACTOR` mirror; restart. | **KEEP** | — |
| A8 | Privacy blur (CSS levels, toggle) | no (W#247, 267) | yes (170 LOC) | NICE | One script, stable tier. | **KEEP** | — |
| A9 | Custom CSS file (covers wallpaper / font override) | no (W#182, 277, 219) | yes | LATER | | **DROP** | dropped |
| A10 | Chat colour themes (14) | no | yes | DROP | | **DROP** | dropped |
| A11 | Buttons injected into WA nav rail | no | yes | DROP | | **DROP** | dropped |
| A12 | Chat-list collapse to avatar strip | no (W#159, 331) | yes (800 LOC) | DROP | | **DROP** | dropped |
| A13 | Muted-updates hide / focus mode / HD AB flag / linked-device name patch | no | yes | DROP | | **DROP** | dropped |
| A14 | Smooth scrolling (`ScrollAnimatorEnabled`) — **setting, default OFF** | no (W#178) | yes | NICE | Owner: expose as option, off by default. | **KEEP** | — (attr default off in skeleton) |
| A15 | Emoji panel dismiss on outside click | no | yes | DROP | | **DROP** | dropped |
| A16 | Undo send | no | yes | DROP | | **DROP** | dropped |
| A17 | HD media auto-click | no (Y#96) | yes | DROP | | **DROP** | dropped |

## 5. Media, permissions, files

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| M1 | Camera / mic / notification / geolocation permissions (`QWebEnginePermission`), persisted, reviewable | dialog inert | works | CORE | Calls (W#287). | **KEEP** | — |
| M2 | Screen-share picker + PipeWire flag | no | yes | CORE | | **KEEP** | — |
| M3 | Call pop-out window kept in-app, closable | orphan page | yes | CORE | W#333. | **KEEP** | — |
| M4 | External links → system browser | yes | yes | CORE | | **KEEP** | done |
| M5 | Native file chooser (portal-aware), last dir | Qt dialog | yes | CORE | W#296, 285. | **KEEP** | — |
| M6 | Drag-and-drop file attach (portal path resolution, threaded read, synthetic paste) | broken (W#111, 285) | yes (520 LOC) | NICE | Cap complexity; Y#32 lesson. | **KEEP** | — |
| M7 | Clipboard image paste rescue | no (W#33, 311) | yes (45 LOC) | CORE | | **KEEP** | — |
| M8 | Downloads: Chromium flow + minimal UI (dir setting, notification with "open folder") | manager window (bugs) | same | CORE (minimal) | Lesson B13. | **KEEP** | — |
| M9 | Mute page audio toggle | yes | yes | CORE | | **KEEP** | — |
| M10 | Media autoplay toggle | inverted | inverted | DROP | `PlaybackRequiresUserGesture=false`. | **DROP** | dropped |
| M11 | Missing-codec / media-stuck notice | no | yes | LATER | Snap/Flatpak runtimes have codecs. | **DROP** | dropped |
| M12a | Certificate-error override dialog | Qt example | same | DROP | Single-origin app: hard fail with message. | **DROP** | dropped |
| M12b | Proxy-auth dialog | Qt example | same | NICE | Needed with P3. | **KEEP** | — |
| M13 | Context-menu filtering | yes | yes | CORE | | **KEEP** | done |

## 6. Privacy, lock, network, performance

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| P1 | App lock, **hardened**: PBKDF2 passcode, covers every window, notifications suppressed + page hidden while locked, attempt throttling; lock on start / hide / idle | base64 `asdfg`, overlay | PBKDF2, overlay, bypassable | NICE | ADR-015. | **KEEP** (M5) | — |
| P2 | WebRTC IP handling policy | no | yes | NICE | One flag. | **KEEP** | — |
| P3 | Network proxy (system / none / HTTP / SOCKS5 + auth) | system only (W#75, 223) | yes | NICE | Password via keychain or not stored. | **KEEP** | — |
| P4 | Autostart at login (XDG / registry), sandbox-aware, starts minimized | no | yes (buggy) | NICE | | **KEEP** | — |
| P5 | Storage manager with safe-delete guard | yes (W#230) | yes | CORE | | **KEEP** | — |
| P6 | Hardware acceleration: auto / on / off | global flag | 18 knobs | CORE (1 setting) | Lesson B4. | **KEEP** | — |
| P7 | Remaining perf knobs | no | yes | DROP | Env var for experts. | **DROP** | dropped |
| P8 | Unload page when hidden for N minutes | no (W#255, 194) | dormancy | LATER | Measure first. | **LATER** | — |
| P9 | Profile export/import | no | yes | DROP | | **DROP** | dropped |
| P10 | Log file + ring buffer + Chromium stderr + "Copy diagnostics" | no | yes | CORE | Lesson B1. | **KEEP** | done |
| P11 | Update checker | none | yes | LATER | | **DROP** | dropped |
| P12 | AppImage self-update | no | never invoked (Y#85) | DROP | | **DROP** | dropped |

## 7. Multi-account

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| X1 | `--profile <name>`: separate storage, settings, instance key | no (W#176) | yes | CORE (plumbing) | Lesson B15. | **KEEP** | done |
| X2 | In-window account tabs (simple strip, ≤ 400 LOC) | no | yes (600 LOC) | LATER | Most-requested #4. | **LATER** | — |
| X3 | Tab tear-off / detached windows / grid view | no | yes (~1 500 LOC) | DROP | | **DROP** | dropped |
| X4 | Per-account unread via IndexedDB | no | yes | DROP | | **DROP** | dropped |

## 8. Spell check & input

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| L1 | Spell check with `.bdic` for the system language (bundled, built from hunspell) | advertised, absent | yes | NICE | ADR-017. | **KEEP** | — |
| L2 | Additional dictionaries on demand (SHA-256 verified), plain list UI | no | yes + painted delegate | LATER | ADR-017; needs hosted dictionary release. | **KEEP** | — |
| L3 | Spell-check focus/cycle shortcut, tray spelling submenu | no | yes | DROP | | **DROP** | dropped |
| L4 | IME plugins (fcitx5/ibus) bundled in snap/flatpak | missing (W#7, 21) | partial | CORE (packaging) | | **KEEP** | — |

## 9. Messaging automation, AI, APIs

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| Z1 | Scheduled messages | no | yes (770 LOC) | DROP | ToS risk, fragile. | **DROP** | dropped |
| Z2 | Quick compose overlay | no | yes | DROP | | **DROP** | dropped |
| Z3 | `--send` CLI, send-by-name, attachment sender, templates | no | yes (~1 000 LOC) | DROP | | **DROP** | dropped |
| Z4 | Auto-reply rules | no | yes | DROP | | **DROP** | dropped |
| Z5 | Local HTTP API, Cloud Business API, webhooks | no | yes (~850 LOC) | DROP | | **DROP** | dropped |
| Z6 | Saved/canned replies | no | yes | DROP | | **DROP** | dropped |
| Z7 | AI assistant, Ollama | no | yes (~800 LOC) | DROP | | **DROP** | dropped |
| Z8 | Inline translation, chat export | no | yes (~780 LOC) | DROP | | **DROP** | dropped |
| Z9 | Custom JS add-ons | no | yes | DROP | | **DROP** | dropped |

## 10. Misc / promo / dead

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| D1 | About dialog (version, Qt/Chromium, install type, "Copy diagnostics") | yes | yes | CORE | W#257. | **KEEP** | done |
| D2 | Rate-app nag | yes | yes | DROP | | **DROP** | dropped |
| D3 | "More apps" carousel | yes | dead | DROP | | **DROP** | dropped |
| D4 | One donate/support link in About | 2 | 3 | NICE | Single link, no nag. | **KEEP** | done |
| D5 | Identicon avatars for notifications (stable hash) | yes | yes | CORE | Part of N1. | **KEEP** | done |

---

## Approved scope at a glance (2026-08-27)

**KEEP — v1 (M1–M4):** S1–S14, S16, S20, S26, T1–T6, N1–N3, N5–N7, N10, N11, A1, A2, A6, A7,
A8, A14 (off by default), M1–M9, M12b, M13, P2–P6, P10, X1, L1, L2, L4, D1, D4, D5.
**KEEP — M5:** P1 (hardened app lock).
**LATER (planned, after v1):** S23, T8, P8, X2.
**DROP:** everything else — no code for these rows will exist in the tree.

Platforms (ADR-016): Linux + Windows. App id (ADR-014): `com.ktechpit.whatsie`.

## Settings surface target

whatsie had ~30 options, whatly ~150. Approved rows imply roughly: close action, start
minimized, tray click, hide tray, monochrome tray, notifications on/off, timeout, sound, DND,
theme, zoom ×2, interface scale, smooth scrolling, privacy blur, download dir, mute, hardware
acceleration, WebRTC policy, proxy, autostart, spell-check languages, app lock — **≈ 24
options** on 4 tabs (General, Appearance, Notifications, Privacy & Advanced). Restart-required:
hardware acceleration, interface scale (2).
