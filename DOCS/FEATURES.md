# Feature diff: whatsie vs whatly vs new-whatsie (proposal)

**Purpose.** This is the contract for scope. Every row needs an owner decision before it is
implemented (ADR-013). Fill the **Decision** column with `KEEP`, `LATER`, or `DROP`; blank means
undecided. `Status` is updated by the implementer (`—` / `wip` / `done` / `dropped`).

Sources: `reference/analysis-whatsie.md` (§3), `reference/analysis-whatly.md` (§3, §9),
`reference/github-issues-2026-08-27.md` (themes, most-requested). **W#** = whatsie issue,
**Y#** = whatly issue.

**Recommendation legend**
- **CORE** — a general-audience WhatsApp wrapper is not usable/credible without it. Proposed for v1.
- **NICE** — cheap, low-maintenance, clearly requested. Proposed for v1 if it stays small, else 1.x.
- **DROP** — niche, high-maintenance, ToS/privacy risk, or depends on WhatsApp internals.

Sizes are the LOC whatly spent on the feature (a proxy for maintenance cost).

---

## 1. Session, window, lifecycle

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| S1 | Persistent WhatsApp Web session (named profile, disk cache/cookies/permissions) | yes (singleton destroyed after QApplication) | yes, per-account | **CORE** | The product. Profile owned by the view (ADR-003). | | done (skeleton) |
| S2 | Window geometry/state persistence (Wayland-safe, screen-aware) | yes, but re-centres on cursor screen every start | yes, tracks normal geometry by hand | **CORE** | W#103, 135 | | done (basic) |
| S3 | Close button → minimize to tray *or* quit (setting) | yes | yes | **CORE** | | | — |
| S4 | Start minimized (setting + `--minimized`) | yes | yes | **CORE** | | | — |
| S5 | Never hide window when no tray is available | no (users lost the window, W#292) | yes | **CORE** | Lesson B7 | | — |
| S6 | Honour session logout/shutdown (`commitDataRequest`) | no (close veto blocked KDE logout, W#66) | yes | **CORE** | | | — |
| S7 | Fullscreen F11 + page-requested fullscreen (video) with reliable exit | yes, flickers; exit broken (W#169, 333) | yes, flickers | **CORE** | Must honour `fullScreenRequested(false)` | | — |
| S8 | Single instance + IPC (raise, open URL, settings, lock, …) | vendored SingleApplication, args joined by spaces | same, lossy `split(" ")` | **CORE** | ADR-010: `QLocalServer` + JSON, profile-aware key | | — |
| S9 | CLI options (`--minimized`, `--profile`, `--log-file`, `--version`, `--new-chat`, …) | 9 options, manual `-h` | ~20 options incl. automation | **CORE** (small set) | Only what maps to a UI action. | | — |
| S10 | `whatsapp://send?phone=` scheme handler + desktop actions | yes (`?`→`&` hack) | yes | **CORE** | Build `/send?phone=` URL in C++ and `load()` it — no JS. | | — |
| S11 | `https://wa.me/`, `api.whatsapp.com`, `chat.whatsapp.com` invite links | no (W#3, 172, 186) | yes (`inviteCodeFromUrl`) | **NICE** | Cheap once S10 exists; register as handler. | | — |
| S12 | Render-process crash recovery with back-off (no infinite dialog loop) | dialog on every crash | loop breaker + optional auto-reload | **CORE** | W#102, Y#28 | | — |
| S13 | Connection watchdog (WebSocket monkey-patch, reload after resume) | no (W#208) | yes, 2 strikes / 3 reloads | **NICE** | Keep the *policy* pure and tested; reload only, never wipe. | | — |
| S14 | Auto-reload when network comes back (`QNetworkInformation`) | no | no | **NICE** | Cheaper alternative/complement to S13. | | — |
| S15 | "Loading quirk" / "Error" title → wipe storage + quit | yes | yes | **DROP** | ADR-011. Replace with a diagnostic page with explicit buttons. | | — |
| S16 | Corrupt service-worker cache recovery (once per page) | no | yes (Y#43) | **NICE** | 25 lines, non-destructive. | | — |
| S17 | Session backup/restore of IndexedDB at startup | no | yes, synchronous copy each launch | **DROP** | Blocks GUI; unencrypted copy of the session. | | — |
| S18 | Low-disk warning + movable data dir | no | yes | **DROP** | Rare; adds a path setting everything must honour. | | — |
| S19 | Restart-in-place (`--restart-wait`) | no | yes | **DROP** | Exists only because ~20 settings "require restart"; aim for ≤ 2 such settings. | | — |
| S20 | Wayland RHI failure → one-shot XCB relaunch; start-up crash escalation | no | yes (Y#3, 84) | **NICE** | Keep only if it stays ~60 lines and is off the happy path. | | — |
| S21 | Setup wizard (3 pages) | no | yes | **DROP** | Nothing to set up; defaults must be right. | | — |
| S22 | Custom frameless title bar + edge resizer | no | yes (400 LOC) | **DROP** | Cosmetic, fights every compositor. | | — |
| S23 | Global hotkey to raise window (portal/X11) | no (TODO) | yes (330 LOC, Linux only) | **LATER** | Requested (W#10, 14); portal-only implementation if ever. | | — |
| S24 | Configurable local shortcuts with conflict detection | no (fixed set) | yes, restart required | **NICE** | W#28, 44, 136. Only if live (no restart). | | — |
| S25 | Command palette (Ctrl+K) | no | yes | **DROP** | Power-user feature over a menu of ~10 actions. | | — |
| S26 | Shortcuts cheat-sheet (Help → Shortcuts) | popup built from action text | yes | **NICE** | W#319. Generate from `QAction` list. | | — |
| S27 | Interface translations (Qt `.ts`) + in-app language picker | `it_IT.ts` never loaded | 22 languages, 21 machine-translated | **NICE** (mechanism) | Ship the mechanism + English; accept community `.ts` files; no MT. W#171, 183, 304. | | — |

## 2. Tray

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| T1 | Tray icon with context menu (show/hide, reload, settings, about, quit) | yes (index-based menu edits) | yes | **CORE** | | | — |
| T2 | Unread badge on tray + window icon | title parsing `(N) WhatsApp`, 10 PNGs | composed image, 1–9 / 99+, tested | **CORE** | Compose badge in code (pure, tested); source count from bridge (W3) with title fallback. | | — |
| T3 | Monochrome / symbolic tray icon option | no (W#93, 128) | yes (Y#14) | **NICE** | Ship SVG symbolic variant. | | — |
| T4 | Left-click toggles window (setting) | yes | yes + Windows focus grace | **CORE** | W#274 | | — |
| T5 | Hide tray icon entirely (setting) | no (W#127, 193) | no | **NICE** | Only with S5 in place. | | — |
| T6 | Connection-state dimmed icon | no (W#108) | yes | **NICE** | Needs S13. | | — |
| T7 | Recent-unread submenu / open chat by name from tray | no | yes (150 LOC, reads IndexedDB) | **DROP** | Depends on WA private DB schema. | | — |
| T8 | Unity launcher badge / `--unread` runtime file | no (W#122, 165) | yes | **LATER** | Cheap if T2 exists; low demand. | | — |

## 3. Notifications

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| N1 | Native Linux notifications via `org.freedesktop.Notifications` (id-tracked, avatar `image-data`, identicon fallback, `desktop-entry`, `category`, click → raise) | libnotify-qt (GPL), id-less ActionInvoked (W#5, 42, 271, 278) | libnotify-qt, correct image hint | **CORE** | ADR-009: own QDBus client, MIT-clean. Fix BGRA/RGBA (W#312, 328). | | — |
| N2 | Windows/macOS notifications via `QSystemTrayIcon::showMessage` | yes (Windows) | yes | **CORE** (when those OSes are in scope) | | | — |
| N3 | XDG portal notifications (Flatpak) | no | yes, cannot detect failure | **NICE** | Only if Flatpak is a v1 target; fix the `NoBlock` issue. | | — |
| N4 | In-app custom popup widget (slide-in, marquee) | yes (default!, leaks) | yes | **DROP** | Native everywhere; W#41, 207/210/212 crashes came from it. | | — |
| N5 | Notification sound hint | no | yes (libnotify path only) | **NICE** | One hint line. | | — |
| N6 | Disable notifications toggle + timeout | yes | yes | **CORE** | | | — |
| N7 | Do-Not-Disturb (manual 1h/2h/indefinite + schedule) | no | yes (240 LOC) | **NICE** | Keep manual DND in tray (small); schedule optional. | | — |
| N8 | Keyword highlights / VIP / muted contacts | no | yes (substring bugs) | **DROP** | WhatsApp has per-chat mute. | | — |
| N9 | Inline reply from notification (KDE/GNOME) | no | yes, via DOM automation | **DROP** | Needs send-by-name automation; ToS/fragility. | | — |
| N10 | Click notification → open that chat | raise only | focus chat via JS | **NICE** | Only via WA's own `Notification.onclick` (the page handles it) — no DOM automation. | | — |
| N11 | Auto-grant web Notification permission | prompt each time (W#307) | auto-grant | **CORE** | | | — |
| N12 | "Started minimized" informational popup | every launch (W#11, 205, 316) | yes | **DROP** | Lesson B6. | | — |
| N13 | Reply reminders (1h/3h/tomorrow) | no | yes | **DROP** | | | — |

## 4. Appearance & zoom

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| A1 | Theme: System / Light / Dark (Qt palette + WA page theme), persisted | light/dark only; reverts (fixed 4×) | + follow system via portal | **CORE** | Lesson B9. Page theme set via WA `localStorage.theme` **before** load; Qt side via `QStyleHints::colorScheme`. | | done (setting) |
| A2 | Live follow-system-theme (xdg portal Settings) | no (W#157) | yes | **CORE** | Qt 6.11 `QStyleHints::colorSchemeChanged` covers most; portal fallback. | | — |
| A3 | Push theme into running page without reload (React fiber walk, WA modules) | yes | yes | **DROP** | Most fragile script in both code bases. Reload on theme change instead (rare action). | | — |
| A4 | Automatic sunrise/sunset theme (geolocation) | yes (DST bug, UB) | yes (680 LOC) | **DROP** | OS does this; QtPositioning dependency gone. | | — |
| A5 | Widget style selector (Fusion/…) | yes | yes | **DROP** | Default to Fusion with palette; system style otherwise. Fewer settings. | | — |
| A6 | Zoom: normal vs maximized factors, Ctrl+/-/0, Ctrl+wheel blocked, min-size scales with zoom | yes (no min-size scaling, W#192) | yes | **CORE** | W#8, 76, 124, 203, 264 (most-requested #3). | | done (factor) |
| A7 | Interface font size / scale factor | no (W#276) | yes | **NICE** | `QT_SCALE_FACTOR` mirror; restart. | | — |
| A8 | Privacy blur (CSS, levels) + toggle | no (W#247, 267) | yes (170 LOC, pure CSS) | **NICE** | Low risk; one script. | | — |
| A9 | Chat wallpaper / custom CSS / web font override | no (W#182, 277, 219) | yes (~400 LOC) | **LATER** | Custom CSS alone (user file) gives all three to power users. | | — |
| A10 | Chat colour themes (14) | no | yes (274 LOC) | **DROP** | Re-parses all stylesheets. | | — |
| A11 | Buttons injected into WA nav rail (theme/blur/zoom) | no | yes (300 LOC) | **DROP** | Geometry-based rail detection. | | — |
| A12 | Chat-list collapse to avatar strip | no (W#159, 331) | yes (800 LOC) | **DROP** | Pixel-exact DOM surgery. | | — |
| A13 | Hide "Muted updates" section / focus mode / HD AB flag / linked-device name patch | no | yes | **DROP** | Text-matching in 10 languages, Meta internals. | | — |
| A14 | Smooth scrolling (`ScrollAnimatorEnabled`) | no (W#178) | yes | **NICE** | One attribute; on by default. | | done (attr) |
| A15 | Emoji panel dismiss on outside click | no | yes | **DROP** | WA behaviour; cosmetic. | | — |
| A16 | Undo send (Enter only) | no | yes | **DROP** | Half-works; WA has delete-for-everyone. | | — |
| A17 | HD media by default (auto-click HD) | no (Y#96 asked to stop the dialog) | yes (MutationObserver) | **DROP** | Body-wide observer; WA now remembers the choice. | | — |

## 5. Media, permissions, files

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| M1 | Camera / mic / notification / geolocation permissions via `QWebEnginePermission`, persisted, UI to review | dialog inert (key mismatch) | works, keyed by enum int | **CORE** | Calls are the #1 requested feature (W#287). | | — |
| M2 | Screen-share picker (`QWebEngineDesktopMediaRequest`) + PipeWire flag | no | yes | **CORE** | Calls/screen share. | | — |
| M3 | Call pop-out window kept in-app (`createWindow` for web.whatsapp.com popups) | orphan page (leak) | yes | **CORE** | W#333 (no way out of pop-out): give the window a close/esc. | | — |
| M4 | External links → system browser; in-domain stays | yes (post-#323) | yes | **CORE** | | | — |
| M5 | Native file chooser (portal-aware) with last-dir memory | Qt dialog default | yes | **CORE** | Sandbox friendliness (W#296, 285). | | — |
| M6 | Drag-and-drop file attach (portal path resolution, threaded read, synthetic paste) | broken (W#111, 285) | yes (520 LOC) | **NICE** | Real user pain; keep but cap complexity. Y#32 lesson. | | — |
| M7 | Clipboard image paste rescue | no (W#33, 311) | yes (45 LOC) | **CORE** | Fixes a real Qt bug users hit constantly. | | — |
| M8 | Downloads: Chromium flow + minimal UI (save to configured dir, notification with "open folder") | Qt-example manager window (bugs) | same + more bugs (inf B/s) | **CORE** (minimal) | Lesson B13; W#273, 301, 336. No separate manager window in v1. | | — |
| M9 | Mute page audio toggle | yes (lost on reload) | yes | **CORE** | | | — |
| M10 | Media autoplay toggle | inverted semantics | inverted | **DROP** | Confusing; default `PlaybackRequiresUserGesture=false`. | | — |
| M11 | Missing-H.264 notice / media-stuck toast | no | yes | **LATER** | Only matters for AppImage/deb builds without codecs; snap/flatpak runtime has codecs. | | — |
| M12 | Certificate-error override dialog, HTTP-auth, client-cert dialogs | Qt example code, double dialog | same | **DROP** (override) / **NICE** (proxy auth) | Single-origin app: cert error = hard fail with message (W#81, 195, 261 loop). Proxy auth only with Q2. | | — |
| M13 | Context-menu filtering (no Back/Forward/Save page) | yes | yes | **CORE** | | | — |

## 6. Privacy, lock, network, performance

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| P1 | App lock (passcode; lock on start / on hide / on idle / on screensaver) | base64 under `asdfg`, overlay only | PBKDF2, still overlay, bypassable (Y#41) | **NICE** | Only with the hardened design (LESSONS A10): PBKDF2, all windows, notifications suppressed, page hidden, attempt throttling. Otherwise DROP. OQ-2. | | — |
| P2 | WebRTC IP handling policy (shield) | no | yes (5 LOC) | **NICE** | One flag. | | — |
| P3 | Network proxy (system / none / HTTP / SOCKS5 + auth) | Qt system proxy only (W#75, 223) | yes (120 LOC, plaintext pw) | **NICE** | `QNetworkProxy` app-wide; password via `QtKeychain` or not stored. | | — |
| P4 | Autostart at login (XDG / registry / LaunchAgent), sandbox-aware | no | yes (buggy Exec) | **NICE** | Common expectation for a messenger. | | — |
| P5 | Storage manager: show sizes, clear cache, clear session (with safe-delete guard) | yes (unit bug; wiped `$HOME` in a branch, W#230) | yes + `isSafeToDelete` | **CORE** | Safe-delete guard is mandatory. | | — |
| P6 | Hardware acceleration: auto / on / off (+ ignore GPU blocklist) | global `--disable-gpu` history | 18 knobs | **CORE** (1 setting) | Lesson B4. Exactly one restart-required setting. | | — |
| P7 | Remaining perf knobs (single-process, per-site, V8 heap cap, cache type/size, font hinting, vsync…) | no | yes | **DROP** | Env var `QTWEBENGINE_CHROMIUM_FLAGS` remains for experts. | | — |
| P8 | Unload page when hidden for N minutes (memory) | no (W#255, 194) | per-account dormancy | **LATER** | Measure first with Qt 6.11; `--js-flags=--max_old_space_size` may be enough. | | — |
| P9 | Profile export/import (tar) | no | yes (blocking, wrong path on Windows) | **DROP** | Users re-link in 30 seconds. | | — |
| P10 | Debug log ring buffer + Chromium stderr capture + "Copy debug info" / bug-report template | no (W#282-style guessing threads) | yes | **CORE** | Lesson B1. `--log-file`, About → Copy diagnostics. | | — |
| P11 | Update checker (GitHub releases, daily) | none | yes, opt-out | **LATER** | Store channels update themselves; opt-in only if added. | | — |
| P12 | AppImage self-update via zsync | no | published, never invoked (Y#85) | **DROP** | | | — |

## 7. Multi-account

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| X1 | `--profile <name>`: separate storage, settings, instance key | no (W#176) | yes (100 LOC) | **CORE** (plumbing) | Lesson B15: parameterise from day one, even without UI. | | — |
| X2 | In-window account tabs (add/rename/remove) | no | yes (600 LOC) | **LATER** | Most-requested #4 (7 issues); only after v1, only if ≤ 400 LOC. | | — |
| X3 | Tab tear-off / detached windows / grid view / layout persistence | no | yes (~1 500 LOC) | **DROP** | Most complex code in whatly for a tiny audience. | | — |
| X4 | Per-account unread via IndexedDB (muted/archived options) | no | yes | **DROP** | WA private schema; use title/bridge count. | | — |

## 8. Spell check & input

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| L1 | Spell check with `.bdic` dictionaries (system language bundled) | advertised, not implemented (W#262) | yes, converted at build | **NICE** | The two most-commented whatsie issues are input-related. OQ-4. | | — |
| L2 | Additional dictionaries on demand (SHA-256 verified) with a plain list UI | no | yes + 410-LOC painted delegate | **LATER** | Plain `QListWidget`. | | — |
| L3 | Spell-check focus/cycle shortcut, tray spelling submenu | no | yes | **DROP** | | | — |
| L4 | IME plugins (fcitx5/ibus) bundled in snap/flatpak | missing (W#7, 21, 63, 233) | partially | **CORE** (packaging) | Packaging task, not code. | | — |

## 9. Messaging automation, AI, APIs

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| Z1 | Scheduled messages (+ recurring, catch-up) | no (W#94, 250) | yes (770 LOC, reloads page to send) | **DROP** | ToS risk, fragile, catch-up burst bug. | | — |
| Z2 | Quick compose overlay (Ctrl+Alt+N) | no | yes | **DROP** | Uses Z1 path; lock bypass (Y#41). | | — |
| Z3 | `--send/--to/--message/--file/--template` CLI, send-by-name, attachment sender, templates | no | yes (~1 000 LOC, "not verified live") | **DROP** | | | — |
| Z4 | Auto-reply rules | no | yes (400 LOC) | **DROP** | | | — |
| Z5 | Local HTTP API, Meta Cloud Business API, webhooks | no | yes (~850 LOC, security holes) | **DROP** | | | — |
| Z6 | Saved/canned replies | no | yes (110 LOC) | **DROP** | WA has none, but this is DOM automation. | | — |
| Z7 | AI assistant (summarise/improve/suggest), Ollama integration | no | yes (~800 LOC, plaintext keys) | **DROP** | Sends transcripts to endpoints; not a wrapper's job. | | — |
| Z8 | Inline translation (LibreTranslate), chat export (scroll-scrape) | no | yes (~780 LOC) | **DROP** | | | — |
| Z9 | Custom JS add-ons per account | no | yes | **DROP** | Security surface, support burden. | | — |

## 10. Misc / promo / dead

| ID | Feature | whatsie | whatly | Rec. | Rationale | Decision | Status |
|---|---|---|---|---|---|---|---|
| D1 | About dialog with version, Qt/Chromium, install type, links, "Copy diagnostics" | yes | yes + bug-report template | **CORE** | W#257. | | — |
| D2 | Rate-app nag dialog | yes | yes (re-arms itself) | **DROP** | | | — |
| D3 | "More apps" snapcraft carousel (network calls on lock screen) | yes | dead code | **DROP** | | | — |
| D4 | Donate buttons (3) | 2 | 3 | **NICE** (1 link in About) | Owner's call; one link max. | | — |
| D5 | Identicon avatar generator for notifications | yes | yes (unstable hash seed) | **CORE** (part of N1) | Stable hash (`qHash` with fixed seed or own FNV). | | — |

---

## Proposed v1 scope at a glance

**CORE (proposed for v1):** S1–S10, S12, T1, T2, T4, N1, N2*, N6, N11, A1, A2, A6, M1–M5, M7, M8, M9, M13, P5, P6, P10, X1, L4, D1, D5.
**NICE (v1 if small, else 1.x):** S11, S13/S14, S16, S20, S24, S26, S27, T3, T5, T6, N3, N5, N7, N10, A7, A8, A14, M6, P1†, P2, P3, P4, L1, D4.
**LATER:** S23, T8, A9, M11, P8, P11, X2, L2.
**DROP:** everything else (≈ 20 k of whatly's 34 k lines).

\* N2 depends on OQ-3 (Windows/macOS in v1?). † P1 only with the hardened design (OQ-2).

## Settings surface target

whatsie had ~30 options, whatly ~150. Target for v1: **≤ 25 options** on 4 tabs
(General, Appearance, Notifications, Privacy & Advanced). Every option must map to a row above.
