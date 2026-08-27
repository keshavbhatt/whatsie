# GitHub issue review: keshavbhatt/whatsie and shakaran/whatly

Generated 2026-08-27 from the GitHub API via `gh`. Raw data lives beside this file:
`issues-whatsie.json`, `issues-whatly.json`, `prs-whatsie.json`, `prs-whatly.json`.

## Repo stats

| | keshavbhatt/whatsie | shakaran/whatly |
|---|---|---|
| Description | Feature rich WhatsApp Client for Desktop Linux | Feature rich WhatsApp Client for Desktop Linux (fork of whatsie) |
| Created | 2021-04-04 | 2026-07-11 |
| Last push | 2026-08-18 | 2026-08-23 |
| Stars / forks | 3365 / 96 | 17 / 2 |
| License | MIT | MIT |
| Archived | no | no |
| Issues (total / open / closed) | 297 / 155 / 142 | 39 / 10 / 29 |
| PRs (total / merged / open / closed) | 40 / 27 / 7 / 6 | 61 / 57 / 2 / 2 |

Notes on the numbers:

- whatsie has 155 open issues, many of them dating back to 2022-2024 and never triaged; its issue tracker was effectively unattended between late 2024 and mid 2026 (#282 "Is the project dead?"). Closures in 2026-03 (v5.0/5.1 Qt6 release) swept a batch of old ones.
- whatly is 7 weeks old; most of its 39 issues are already closed, and a large share were filed by one very thorough contributor (`gbmaizol`) who writes packaging audits with measurements. Its 57 merged PRs in 7 weeks show rapid iteration on the fork.
- Issue-number gaps in the tables below are pull requests (GitHub shares the number space).

## Category definitions

bug, crash, feature-request, packaging/distro, webengine/whatsapp-breakage, notifications, tray/window, login/session, media (camera/mic/audio/video/screenshare), downloads, theme/ui, performance, security/privacy, spellcheck/i18n, docs/question, other.
"bug" is used for issues that don't fit a narrower bucket; GPU/rendering failures are filed under "bug" with a "GPU:" prefix in the gist so they can be found.

## Category breakdown

### whatsie (total / open)

| Category | Total | Open |
|---|---|---|
| feature-request | 46 | 29 |
| packaging/distro | 32 | 14 |
| tray/window | 29 | 15 |
| crash | 27 | 11 |
| login/session | 27 | 14 |
| notifications | 24 | 9 |
| theme/ui | 19 | 11 |
| webengine/whatsapp-breakage | 19 | 9 |
| spellcheck/i18n | 18 | 9 |
| performance | 15 | 8 |
| bug | 14 | 13 |
| downloads | 7 | 2 |
| docs/question | 6 | 2 |
| other | 6 | 4 |
| security/privacy | 6 | 3 |
| media | 2 | 2 |

### whatly (total / open)

| Category | Total | Open |
|---|---|---|
| packaging/distro | 10 | 3 |
| bug | 5 | 3 |
| docs/question | 4 | 1 |
| media | 4 | 2 |
| notifications | 4 | 0 |
| crash | 3 | 0 |
| login/session | 2 | 1 |
| security/privacy | 2 | 0 |
| theme/ui | 2 | 0 |
| performance | 1 | 0 |
| spellcheck/i18n | 1 | 0 |
| tray/window | 1 | 0 |

## All issues: keshavbhatt/whatsie (297)

| # | State | Title | Category | Gist | Created | Closed | Comments | URL |
|---|---|---|---|---|---|---|---|---|
| 338 | OPEN | [Bug] Unlock to access Settings | security/privacy | App lock blocks Settings with "Unlock to access Settings" on Flatpak, no unlock prompt | 2026-08-26 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/338 |
| 336 | OPEN | Can't download document | downloads | Cannot download a document (snap 5.1.0, Qt 6.11) | 2026-08-22 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/336 |
| 333 | OPEN | no way out when full screen pop-out window (quit app needed) | tray/window | Full-screen pop-out (video) window has no exit; must quit app | 2026-08-04 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/333 |
| 332 | OPEN | flatpak 50/50 picking up dark theme | theme/ui | Flatpak picks up dark theme only ~50% of launches (GNOME Wayland) | 2026-08-04 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/332 |
| 331 | OPEN | option to minimize side bar | feature-request | Option to collapse/minimize the chat sidebar (like Telegram) | 2026-07-24 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/331 |
| 330 | OPEN | [IT IS NOT AN ISSUE] Thank you 👍 | other | Thank-you note: only client where video calls work | 2026-07-22 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/330 |
| 329 | OPEN | Notifications are only in the Whatsie window. | notifications | Notifications rendered only inside the app window, not system-wide (Niri/wlroots) | 2026-07-19 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/329 |
| 328 | CLOSED | Colors inverted on photos on native notifications on Linux Mint 22.8 | notifications | Contact photo colours inverted in native notifications (Mint 22.8) - RGB/BGR swap | 2026-07-17 | 2026-07-18 | 2 | https://github.com/keshavbhatt/whatsie/issues/328 |
| 327 | OPEN | Two accounts, two windows, zero mix-ups - offering to build multi-account support | feature-request | Offer to build side-by-side multi-account windows (colour-tagged) | 2026-07-16 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/327 |
| 325 | OPEN | Proposal: free code signing for future Windows releases (SignPath Foundation) | packaging/distro | Proposal: free SignPath code signing for Windows builds | 2026-07-13 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/325 |
| 320 | OPEN | Whatsie doesn't launch (Qt related issue) | packaging/distro | Fails to launch: xcb-cursor0 missing so Qt xcb platform plugin cannot load | 2026-07-05 | - | 25 | https://github.com/keshavbhatt/whatsie/issues/320 |
| 319 | OPEN | What are the in app shortcuts? | docs/question | Which in-app keyboard shortcuts exist / how to focus elements | 2026-06-27 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/319 |
| 318 | OPEN | whatsie does not load whatsapp web page | webengine/whatsapp-breakage | WhatsApp Web page no longer loads at all since 2026-06-20 (openSUSE rpm 4.16.3) | 2026-06-22 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/318 |
| 317 | CLOSED | Dark theme not remembered (Flatpak) | theme/ui | Dark theme reverts to light on every restart (Flatpak) | 2026-06-02 | 2026-06-11 | 3 | https://github.com/keshavbhatt/whatsie/issues/317 |
| 316 | OPEN | Feature Request: Add option to disable "started minimized in tray" popup notification | notifications | Option to disable "started minimized in tray" popup notification | 2026-05-20 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/316 |
| 315 | CLOSED | Instead of whatsapp its facebook login page | webengine/whatsapp-breakage | Facebook login page shown instead of WhatsApp after idle (AUR 5.1.0) | 2026-05-19 | 2026-05-22 | 5 | https://github.com/keshavbhatt/whatsie/issues/315 |
| 314 | OPEN | Unable to build | packaging/distro | Cannot build on Debian 13: no Makefile / qmake target after CMake migration | 2026-05-18 | - | 10 | https://github.com/keshavbhatt/whatsie/issues/314 |
| 313 | OPEN | "You received a message on your phone, but your version of WhatsApp Web doesn't support it" | webengine/whatsapp-breakage | "Your version of WhatsApp Web doesn't support it" on some messages | 2026-05-11 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/313 |
| 312 | OPEN | Chat picture in notification has colors altered (Fedora GNOME) | notifications | Chat avatar colours altered/inverted in native GNOME notifications since 5.1 | 2026-05-04 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/312 |
| 311 | OPEN | Enable pasting images from clipboard | bug | Pasting an image from clipboard into a chat does nothing | 2026-04-29 | - | 7 | https://github.com/keshavbhatt/whatsie/issues/311 |
| 310 | CLOSED | App is in Portuguese (fixed itself) | spellcheck/i18n | App started in Portuguese despite English system locale | 2026-04-28 | 2026-04-28 | 1 | https://github.com/keshavbhatt/whatsie/issues/310 |
| 307 | OPEN | "Message notifications are off. Turn on" | notifications | "Message notifications are off. Turn on" - web permission prompt has no UI in v5 Flatpak | 2026-04-15 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/307 |
| 306 | OPEN | Error 429 | webengine/whatsapp-breakage | HTTP 429 (rate limited) from WhatsApp Web | 2026-04-09 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/306 |
| 305 | OPEN | Window won't back to previous state from Maximized | tray/window | Restore-down from maximized keeps the window maximized | 2026-04-08 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/305 |
| 304 | OPEN | Translation | spellcheck/i18n | How to change UI language (Italian/Romanian) - does not follow system locale | 2026-04-06 | - | 4 | https://github.com/keshavbhatt/whatsie/issues/304 |
| 303 | CLOSED | Certificate Expired | webengine/whatsapp-breakage | ERR_CERT_DATE_INVALID - server certificate expired page | 2026-04-01 | 2026-04-02 | 2 | https://github.com/keshavbhatt/whatsie/issues/303 |
| 301 | CLOSED | Don't know where Files are saved | downloads | Default download folder changed in v5; cannot open download dir from download window | 2026-03-31 | 2026-03-31 | 0 | https://github.com/keshavbhatt/whatsie/issues/301 |
| 300 | OPEN | Possible fix for: Checkboxes invisible in dark mode on Permissions dialog | theme/ui | Checkboxes invisible in dark mode on Permissions dialog (hardcoded icon) | 2026-03-30 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/300 |
| 299 | OPEN | App does not remember selected theme after restart | theme/ui | Selected theme not persisted across restarts in latest version (regression) | 2026-03-29 | - | 7 | https://github.com/keshavbhatt/whatsie/issues/299 |
| 297 | CLOSED | Saving Images | downloads | Saved images no longer appear in Downloads folder | 2026-03-28 | 2026-03-29 | 5 | https://github.com/keshavbhatt/whatsie/issues/297 |
| 296 | CLOSED | v5.0.0: file picker cannot browse home directory | packaging/distro | Snap v5.0.0 file picker cannot browse home dir - `home` plug removed | 2026-03-28 | 2026-03-29 | 4 | https://github.com/keshavbhatt/whatsie/issues/296 |
| 293 | CLOSED | Missing rounded corners on icons and buttons across the entire UI | webengine/whatsapp-breakage | No rounded corners on avatars/buttons (old Chromium in Qt5 WebEngine) | 2026-03-15 | 2026-03-28 | 3 | https://github.com/keshavbhatt/whatsie/issues/293 |
| 292 | OPEN | WhatSie doesn't restore from tray. | tray/window | Cannot restore from tray; "Restore" menu item greyed out (Fedora Wayland Flatpak) | 2026-03-10 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/292 |
| 291 | OPEN | Middle-clicking a link crashes whatsie | crash | Middle-clicking a link crashes the app (snap 4.16.3) | 2026-03-08 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/291 |
| 290 | CLOSED | The dark mode theme doesn't work properly. | theme/ui | Dark theme applied but chat list stays white (snap) | 2026-03-07 | 2026-03-29 | 2 | https://github.com/keshavbhatt/whatsie/issues/290 |
| 288 | CLOSED | Emoji skin tone picker popup renders behind chat area (z-index issue) | theme/ui | Emoji skin-tone popup renders behind chat area (z-index in injected CSS) | 2026-02-08 | 2026-02-08 | 2 | https://github.com/keshavbhatt/whatsie/issues/288 |
| 287 | OPEN | Whatsapp Video and Voice Call does not worked | media | Voice/video calls: device-selection popup never appears, call does not start | 2026-02-05 | - | 10 | https://github.com/keshavbhatt/whatsie/issues/287 |
| 286 | OPEN | ldconfig failed, exit status 256 | packaging/distro | Cannot launch: "ldconfig failed, exit status 256" (Ubuntu 24.04 snap) | 2026-02-04 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/286 |
| 285 | OPEN | Drag and Drop attachments not working | bug | Drag-and-drop attach says "document has no content"; file dialog cannot see folders (sandbox) | 2026-02-03 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/285 |
| 284 | OPEN | No desktop notifictions on Ubuntu | notifications | Snap on Ubuntu 25.10: only tray notification, nothing in GNOME shell | 2026-01-26 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/284 |
| 283 | CLOSED | White window on start | webengine/whatsapp-breakage | White blank window on start; reload does nothing | 2026-01-23 | 2026-03-31 | 8 | https://github.com/keshavbhatt/whatsie/issues/283 |
| 282 | OPEN | Is the project dead? | docs/question | "Is the project dead?" - no commits for ~2 years | 2026-01-01 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/282 |
| 281 | OPEN | About text | other | About text cannot be changed | 2025-12-16 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/281 |
| 280 | OPEN | app does not load whatsapp messages on start-up | webengine/whatsapp-breakage | "Unexpected error occurred" with retry loop on startup (snap 4.16.3, Dec 2025) | 2025-12-03 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/280 |
| 279 | CLOSED | Whatsie renders User avatars as rectangles instead of circles in the chat list and Status | webengine/whatsapp-breakage | Avatars rendered as rectangles instead of circles (old Chromium) | 2025-11-22 | 2026-03-29 | 6 | https://github.com/keshavbhatt/whatsie/issues/279 |
| 278 | CLOSED | Whatsie opens itself when a non-related notification is clicked on. | notifications | App raises itself when any unrelated notification is clicked | 2025-11-22 | 2026-04-23 | 5 | https://github.com/keshavbhatt/whatsie/issues/278 |
| 277 | OPEN | Feature Request | feature-request | Use a local image as chat wallpaper | 2025-11-19 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/277 |
| 276 | OPEN | The UI ignores system settings | theme/ui | Ignores system font size (stays at 10pt when system is 14pt) | 2025-11-15 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/276 |
| 275 | OPEN | Whatsie doesn't start after last update Arch Linux today | packaging/distro | AUR build breaks after libicu bump (libicui18n.so.76 missing) | 2025-11-11 | - | 6 | https://github.com/keshavbhatt/whatsie/issues/275 |
| 274 | OPEN | Main window doesn't open when clicking on Whatsie tray icon. | tray/window | Left-click on tray icon does nothing on KDE Plasma (Flatpak); must right-click > Restore | 2025-10-12 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/274 |
| 273 | OPEN | Download directory settings not available | downloads | Cannot find setting to change download directory | 2025-10-12 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/273 |
| 272 | OPEN | Failed to start on Fedora 42 (flatpak) | crash | Flatpak crashes on launch on Fedora 42 (QSocketNotifier/qgnomeplatform) | 2025-10-01 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/272 |
| 271 | OPEN | Notification problem | notifications | Clicking a Thunderbird notification opens Whatsie main window | 2025-09-23 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/271 |
| 270 | OPEN | Doesn't open after synching messages | webengine/whatsapp-breakage | Does not open after sync reaches 100% | 2025-09-22 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/270 |
| 269 | CLOSED | Crashing when sending message | crash | SIGABRT: V8 heap exhausted (~3.8 GB) while sending a message | 2025-09-19 | 2025-10-02 | 0 | https://github.com/keshavbhatt/whatsie/issues/269 |
| 268 | OPEN | KDE Neon 6 - kwin crash when trying to play video full screen | media | KWin crashes when playing video fullscreen (NVIDIA Wayland) | 2025-09-04 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/268 |
| 267 | OPEN | [Feature Request] Option to blur/hide messages and other content | feature-request | Option to blur/hide chats and messages for privacy | 2025-09-02 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/267 |
| 264 | OPEN | Configurable page zoom factor | feature-request | How to set a configurable page zoom factor | 2025-08-29 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/264 |
| 263 | OPEN | call function | feature-request | Add voice/video calling | 2025-08-28 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/263 |
| 262 | OPEN | Spell Checker doesn't Work | spellcheck/i18n | Spell checker does not work (Ubuntu) | 2025-08-27 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/262 |
| 261 | OPEN | Server's certificate is not trusted | webengine/whatsapp-breakage | Recurring "Server's certificate is not trusted" popup (Fedora snap) | 2025-08-14 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/261 |
| 260 | OPEN | warning: `whatsie' uses wireless extensions which will stop working for Wi-Fi 7 hardware; use nl80211 | other | dmesg: uses deprecated wireless extensions (Chromium network probe) | 2025-08-14 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/260 |
| 259 | OPEN | Adding option | feature-request | Support several WhatsApp accounts | 2025-08-12 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/259 |
| 257 | OPEN | Enhancement: Add a Help menu with an "About" option that shows the version number | feature-request | Help/About menu with version number and links | 2025-07-07 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/257 |
| 256 | CLOSED | Can't link to phone- Neither QR code or numeric code are displayed | login/session | Neither QR code nor phone-number code displayed at login (AUR 4.16.3) | 2025-07-07 | 2025-07-21 | 2 | https://github.com/keshavbhatt/whatsie/issues/256 |
| 255 | OPEN | Whatsie consumes 9 GB of RAM on Ubuntu (memory leak or excessive usage) | performance | 9 GB RAM usage while idle (snap 4.16.3) | 2025-06-14 | - | 8 | https://github.com/keshavbhatt/whatsie/issues/255 |
| 254 | OPEN | whatsapp | other | Empty issue titled "whatsapp" | 2025-06-09 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/254 |
| 253 | CLOSED | Whatsie (snap) crashes immediately on app launch | crash | Snap crashes immediately on launch (AppArmor/D-Bus denial) | 2025-05-30 | 2026-03-29 | 4 | https://github.com/keshavbhatt/whatsie/issues/253 |
| 252 | OPEN | After start Content of main window doesnt load | bug | GPU: main content never loads, EGL context failures (Flatpak) | 2025-05-27 | - | 4 | https://github.com/keshavbhatt/whatsie/issues/252 |
| 251 | OPEN | It freezed my heart | bug | App froze permanently after a week of use | 2025-05-13 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/251 |
| 250 | OPEN | [BE] Pseudo-programmed Messages | feature-request | Scheduled/timed messages | 2025-05-08 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/250 |
| 249 | OPEN | Whatsie Stuck at 'Loading Your Chats [100%]' on Fedora | login/session | Stuck at "Loading your chats [100%]" (Fedora), 20 comments | 2025-05-07 | - | 20 | https://github.com/keshavbhatt/whatsie/issues/249 |
| 248 | OPEN | Does not open on Fedora 41 | crash | Crashes and closes when sync reaches 100% (Fedora 41 Flathub, NVIDIA) | 2025-04-27 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/248 |
| 247 | CLOSED | Blur Chats & Messages | feature-request | Blur chats & messages like browser extension | 2025-04-26 | 2025-04-30 | 2 | https://github.com/keshavbhatt/whatsie/issues/247 |
| 246 | OPEN | Regression: Snap revision 165 breaks spellchecking on Ubuntu 24.04 | spellcheck/i18n | Snap rev 165 breaks spellchecking on Ubuntu 24.04 (dictionary path / D-Bus) | 2025-04-22 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/246 |
| 245 | OPEN | Whatsie crashing when trying to link device | crash | Crashes (and takes down device) when scanning QR on Raspberry Pi | 2025-04-18 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/245 |
| 244 | OPEN | It crashes constantly | crash | Crashes constantly on Ultramarine 41 KDE Wayland | 2025-04-16 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/244 |
| 243 | CLOSED | No longer opens on Ubuntu 22.04 LTS | login/session | Hangs at "loading chats 100%" on Ubuntu 22.04 snap; AppArmor denials | 2025-04-09 | 2025-04-10 | 2 | https://github.com/keshavbhatt/whatsie/issues/243 |
| 242 | OPEN | Doesn't hide muted status updates | feature-request | Hide muted status updates (ZapZap does) | 2025-04-04 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/242 |
| 241 | OPEN | Big memory consumption. 13X of Windows version. | performance | 800 MB RAM vs 60 MB for Windows client | 2025-04-01 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/241 |
| 240 | OPEN | Says "Unlock to access Settings" not showing unlock window | security/privacy | App lock: "Unlock to access Settings" but unlock window never shows; QR not shown with lock on start | 2025-03-31 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/240 |
| 239 | OPEN | Best WhatSie Integration for Linux Smartphone... | feature-request | Better layout on PinePhone / Linux smartphone screens | 2025-03-22 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/239 |
| 238 | CLOSED | Messages showing "Waiting for this message. This may take a while" | webengine/whatsapp-breakage | "Waiting for this message. This may take a while" on messages that browser shows fine | 2025-03-18 | 2025-03-21 | 2 | https://github.com/keshavbhatt/whatsie/issues/238 |
| 237 | OPEN | different curson theme then system cursor theme | theme/ui | Uses its own cursor theme rather than system cursor (Wayland) | 2025-02-22 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/237 |
| 236 | CLOSED | Change user agent / UA | docs/question | How to change user agent to Firefox/Edge | 2025-01-30 | 2025-02-05 | 2 | https://github.com/keshavbhatt/whatsie/issues/236 |
| 235 | OPEN | Full width mode don't work on Ubuntu 24.04 LTS | theme/ui | Full-width mode does not work on Ubuntu 24.04 (Wayland and X11) | 2025-01-30 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/235 |
| 234 | OPEN | Flatpak refuses to launch | crash | Flatpak opens briefly then crashes: GPU shared context failure + Wayland protocol error | 2025-01-21 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/234 |
| 233 | OPEN | fcitx5 doesn't work on Ubuntu snap | spellcheck/i18n | fcitx5 input method does not work in snap | 2025-01-13 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/233 |
| 232 | OPEN | Touchpad scrolling not work | bug | Touchpad scrolling does not work | 2025-01-09 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/232 |
| 231 | OPEN | several whatsapp accounts? | feature-request | Run several accounts / instances with different profiles | 2025-01-06 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/231 |
| 230 | OPEN | Accidental user home directory deletion  | bug | Cache-clear button wiped the user's home directory (qt6 branch) | 2025-01-03 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/230 |
| 229 | OPEN | Stops connecting to Whatsapp after a while | login/session | Connection to WhatsApp breaks after 1-2 weeks; only logout/relink helps | 2024-12-23 | - | 8 | https://github.com/keshavbhatt/whatsie/issues/229 |
| 228 | OPEN | AppImage support | packaging/distro | AppImage build requested | 2024-12-22 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/228 |
| 227 | OPEN | Flickering window in under Wayland with flatpak on Ubuntu | tray/window | Window flickers/vanishes under Wayland with Flatpak | 2024-12-21 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/227 |
| 226 | OPEN | Removing Whatsie causes /var/log/syslog to grow indefinitely | packaging/distro | After snap removal, syslog grows 10 MB/s with netlink errors | 2024-12-17 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/226 |
| 225 | OPEN | Repeatedly vanishing from tray on openSUSE Leap 15.5 | crash | Silently vanishes from tray ~4 times in 4 hours (openSUSE) | 2024-12-10 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/225 |
| 223 | OPEN | "whatsie -s" - The site can't be reached. | bug | Cannot open Settings (proxy config) without internet; "site can't be reached" | 2024-12-02 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/223 |
| 222 | OPEN | MESA-LOADER: glx: failed to open zink: driver not built! | packaging/distro | MESA-LOADER zink warning on Gentoo | 2024-12-02 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/222 |
| 221 | OPEN | make whatsie run at max refresh rate | feature-request | Run at monitor's max refresh rate instead of 60 Hz | 2024-11-23 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/221 |
| 220 | OPEN | Issue Syncing Chats | login/session | Stuck at "syncing messages ~95%" with 10-20% CPU | 2024-11-20 | - | 7 | https://github.com/keshavbhatt/whatsie/issues/220 |
| 219 | OPEN | Option to change font family | feature-request | Option to change font family | 2024-11-20 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/219 |
| 218 | OPEN | Calling System | feature-request | Add calling like Windows/macOS clients | 2024-11-15 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/218 |
| 217 | OPEN | Right Click Issue | bug | Right-click message options (copy/reply/share) missing | 2024-11-15 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/217 |
| 216 | OPEN | add security flags under a separate configure option | packaging/distro | Hardening compiler flags should be behind a configure option (Gentoo) | 2024-11-04 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/216 |
| 213 | CLOSED | Qt6.2 build issue unable to find qwebengine_convert_dict | packaging/distro | Qt 6.2 build: qwebengine_convert_dict not found | 2024-11-01 | 2024-11-01 | 0 | https://github.com/keshavbhatt/whatsie/issues/213 |
| 212 | CLOSED | Segafults in 4.16.2 | crash | SIGSEGV in QWidget::setMinimumWidth from notification presenter lambda (4.16.2) | 2024-10-31 | 2024-11-01 | 2 | https://github.com/keshavbhatt/whatsie/issues/212 |
| 211 | CLOSED | WhatSie chrashes repeatedly after Fedora KDE upgrade from 40 to 41 | crash | Repeated crashes after Fedora KDE 40->41 upgrade | 2024-10-31 | 2024-11-01 | 3 | https://github.com/keshavbhatt/whatsie/issues/211 |
| 210 | CLOSED | [Ubuntu 24.04]  Whatsie crashes when process turns to  background | crash | Crashes when process goes to background (Ubuntu 24.04, new release) | 2024-10-31 | 2024-11-01 | 10 | https://github.com/keshavbhatt/whatsie/issues/210 |
| 208 | OPEN | lost connection after sleep | login/session | No reconnect after resume from sleep; needs manual reload | 2024-10-28 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/208 |
| 207 | CLOSED | Ubuntu notifications | crash | App closes itself when a message arrives while minimized (Ubuntu) | 2024-10-27 | 2024-11-01 | 11 | https://github.com/keshavbhatt/whatsie/issues/207 |
| 206 | CLOSED | Chat Display Theme resets after every restart (Linux Mint) | theme/ui | Dark theme resets to light on every restart (Mint 22) | 2024-10-27 | 2024-10-27 | 4 | https://github.com/keshavbhatt/whatsie/issues/206 |
| 205 | CLOSED | Feature request: option to disable Reduced Whatsie notifications | notifications | Option to disable "started minimized" popup on every start | 2024-10-27 | 2026-07-23 | 2 | https://github.com/keshavbhatt/whatsie/issues/205 |
| 204 | OPEN | Flatpack size from software manager LM | packaging/distro | Mint software manager shows Flatpak as 1.5 GB download | 2024-10-23 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/204 |
| 203 | OPEN | Running Nobara 40 Wayland on 4k resolution fonts are so small  | theme/ui | Fonts tiny on 4K Wayland; --force-device-scale-factor rejected as unknown option | 2024-10-20 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/203 |
| 201 | OPEN | Whatsie don't load messages. | webengine/whatsapp-breakage | Does not load messages | 2024-10-12 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/201 |
| 200 | OPEN | discrete graphics | bug | GPU: blank screen when BIOS set to discrete NVIDIA only | 2024-09-22 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/200 |
| 198 | CLOSED | Notification doesn't appear on Ubuntu 24.04 LTS when using the Native option | notifications | Native notification does not pop up on Ubuntu 24.04; custom does not go to notification bay | 2024-09-16 | 2024-10-19 | 4 | https://github.com/keshavbhatt/whatsie/issues/198 |
| 196 | OPEN | It's not working on Ubuntu 24.04.1 LTS | login/session | Stuck "loading chats" forever with CPU spinning (snap, Ubuntu 24.04) | 2024-09-14 | - | 13 | https://github.com/keshavbhatt/whatsie/issues/196 |
| 195 | OPEN | Server certificate is not trusted. | webengine/whatsapp-breakage | "Server certificate is not trusted" (Fedora 40 snap) | 2024-09-12 | - | 7 | https://github.com/keshavbhatt/whatsie/issues/195 |
| 194 | OPEN | Broken with memory leak | performance | Memory leak: 12 GB RAM, 10% CPU, cannot send | 2024-09-02 | - | 7 | https://github.com/keshavbhatt/whatsie/issues/194 |
| 193 | OPEN | Add option to hide system tray icon | tray/window | Option to hide system tray icon | 2024-08-18 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/193 |
| 192 | OPEN | When a scaling factor ("Zoom factor when normal" in the settings) is set, the minimum window size is not multiplied by the scale factor, causing the window to be forced to be too large | tray/window | Minimum window size not multiplied by zoom factor | 2024-08-13 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/192 |
| 191 | OPEN | [FR] Disable GPU acceleration and/or reduce memory usage. | performance | Option to disable GPU acceleration / reduce VRAM usage | 2024-08-06 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/191 |
| 190 | OPEN | Abnormally High CPU Thrashing | performance | Persistent high CPU thrashing on Core2Duo (18 comments) | 2024-08-05 | - | 18 | https://github.com/keshavbhatt/whatsie/issues/190 |
| 189 | OPEN | whatsie on Wayland | tray/window | Runs under XWayland; native Wayland requested | 2024-08-04 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/189 |
| 187 | OPEN | [Feature Request] Spoof CLient to Whatsapp for MacOS to get voice and videocall support | feature-request | Spoof macOS user-agent to unlock voice/video calls | 2024-08-01 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/187 |
| 186 | OPEN | how to open group chat invite links on Whatsie? | feature-request | Group invite links from browser do nothing in Whatsie | 2024-07-31 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/186 |
| 185 | OPEN | OOM? Whatsie performs several GCs and interrupts then | crash | V8 OOM: repeated GCs at ~3.8 GB then process dies | 2024-07-30 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/185 |
| 184 | OPEN | Whatsie receives messages but cannot send them  | login/session | Receives messages but cannot send (snap & Flatpak, Ubuntu 24.04) | 2024-07-26 | - | 13 | https://github.com/keshavbhatt/whatsie/issues/184 |
| 183 | OPEN | Need A Translation | spellcheck/i18n | Request for multi-language UI translation | 2024-07-16 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/183 |
| 182 | OPEN | Support for custom theming with provided css | feature-request | Custom CSS/user-styles support (catppuccin) | 2024-07-09 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/182 |
| 181 | OPEN | Can't sent message when Whatsie and Mobile app are used at the same time  | login/session | Cannot send after opening mobile app simultaneously | 2024-06-21 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/181 |
| 180 | CLOSED | How to disable notification preview? | notifications | Option to disable notification preview removed in 4.15 | 2024-06-20 | 2024-10-19 | 2 | https://github.com/keshavbhatt/whatsie/issues/180 |
| 179 | CLOSED | Call for Community Contribution: Create a New Icon for Whatsie! | other | Call for community icon (trademark issue with WhatsApp icon on Flathub) | 2024-06-08 | 2024-08-01 | 1 | https://github.com/keshavbhatt/whatsie/issues/179 |
| 178 | OPEN | Implement smooth scrolling | feature-request | Smooth scrolling option | 2024-06-06 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/178 |
| 177 | CLOSED | Ubuntu 24.04 Apparmor issue | packaging/distro | Ubuntu 24.04 AppArmor blocks snap D-Bus; cannot send messages | 2024-06-06 | 2024-06-08 | 2 | https://github.com/keshavbhatt/whatsie/issues/177 |
| 176 | OPEN | when using alternative home for multi instances of whatsie cannot run 2 at the same time | feature-request | Second instance with alternate HOME just focuses the first (single-instance guard) | 2024-05-30 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/176 |
| 175 | OPEN | WhatSie 4.15.0 segfaults on startup | crash | Segfault on startup 4.15.0 (AUR/Manjaro) | 2024-05-26 | - | 4 | https://github.com/keshavbhatt/whatsie/issues/175 |
| 174 | CLOSED | WhatSie keeps shutting down | crash | Shuts down by itself every 10-20 min (Flatpak, Mint) | 2024-05-26 | 2024-05-28 | 6 | https://github.com/keshavbhatt/whatsie/issues/174 |
| 173 | OPEN | Icon | tray/window | How to change tray icon appearance | 2024-05-07 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/173 |
| 172 | CLOSED | URL Handler Integration | feature-request | URL handler for wa.me / api.whatsapp.com links | 2024-04-19 | 2024-05-25 | 3 | https://github.com/keshavbhatt/whatsie/issues/172 |
| 171 | OPEN | Dispaly Language - how to customize? | spellcheck/i18n | How to change display language | 2024-04-13 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/171 |
| 170 | CLOSED | Stuck on "Loading your messages" | login/session | Periodically stuck on "Loading your messages" until relink | 2024-04-12 | 2024-05-25 | 9 | https://github.com/keshavbhatt/whatsie/issues/170 |
| 169 | CLOSED | Unable to exit fullscreen mode | tray/window | Cannot exit fullscreen after fullscreen video | 2024-04-04 | 2024-05-25 | 3 | https://github.com/keshavbhatt/whatsie/issues/169 |
| 168 | CLOSED | Please port to Qt6 | packaging/distro | Port to Qt6 (Gentoo downstream) | 2024-03-24 | 2026-07-14 | 3 | https://github.com/keshavbhatt/whatsie/issues/168 |
| 167 | CLOSED | Segfault leads to complete system freeze | crash | Segfault in tray leads to full system freeze (Mint) | 2024-03-21 | 2024-05-25 | 3 | https://github.com/keshavbhatt/whatsie/issues/167 |
| 166 | OPEN | Account switching? | feature-request | Account switching between phone numbers | 2024-03-16 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/166 |
| 165 | OPEN | Get number of unread messages from CLI | feature-request | Get unread count from CLI | 2024-02-21 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/165 |
| 164 | CLOSED | Only Fullscreen | tray/window | Only fullscreen; no windowed mode (Debian) | 2024-02-18 | 2024-05-25 | 3 | https://github.com/keshavbhatt/whatsie/issues/164 |
| 163 | CLOSED | Excessive CPU usage | performance | Excessive CPU usage | 2024-02-14 | 2024-05-25 | 6 | https://github.com/keshavbhatt/whatsie/issues/163 |
| 162 | OPEN | Whatsie sync message conversation | login/session | Messages sent from Whatsie never reach contact or phone | 2024-02-09 | - | 6 | https://github.com/keshavbhatt/whatsie/issues/162 |
| 161 | OPEN | Native notification does not work on ubuntu | notifications | Native notifications do not pop up on Ubuntu | 2024-01-21 | - | 4 | https://github.com/keshavbhatt/whatsie/issues/161 |
| 160 | OPEN | Native Notification doesnt work with wired on Arch Linux | notifications | Native notifications do not work with `wired` daemon on Arch | 2024-01-17 | - | 4 | https://github.com/keshavbhatt/whatsie/issues/160 |
| 159 | CLOSED | Hide sidebar when window is narrow | feature-request | Collapse sidebar when window is narrow (wontfix) | 2024-01-15 | 2024-05-25 | 2 | https://github.com/keshavbhatt/whatsie/issues/159 |
| 157 | OPEN | system theme gnome | theme/ui | Does not follow GNOME system theme on Wayland | 2024-01-09 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/157 |
| 156 | CLOSED | Spell checker doesn't show languages | spellcheck/i18n | Spell checker shows no languages after building from source | 2024-01-08 | 2024-10-10 | 3 | https://github.com/keshavbhatt/whatsie/issues/156 |
| 155 | OPEN | snap doesn't start | crash | Snap does not start: libGL fbConfigs / radeonsi driver errors | 2024-01-04 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/155 |
| 154 | OPEN | Nothing but a blank screen on Wayland | bug | GPU: nothing but a blank screen on Wayland (NVIDIA), 14 comments | 2024-01-03 | - | 14 | https://github.com/keshavbhatt/whatsie/issues/154 |
| 153 | OPEN | Multiple client stuck | login/session | Same account on second PC leaves Whatsie stuck on "Loading your chats" | 2023-12-21 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/153 |
| 152 | CLOSED | error launching whatsie on kde plasma wayland | crash | Fails on KDE Plasma 6 Wayland: cookie sqlite database locked | 2023-12-04 | 2024-05-25 | 2 | https://github.com/keshavbhatt/whatsie/issues/152 |
| 151 | CLOSED | Blank window, but sort-of-working | webengine/whatsapp-breakage | Blank window after login on Arch (Nov 2023) though notifications work | 2023-12-01 | 2023-12-03 | 10 | https://github.com/keshavbhatt/whatsie/issues/151 |
| 149 | CLOSED | Blank screen | webengine/whatsapp-breakage | Sudden blank screen after loading (snap Debian 11) | 2023-11-30 | 2023-11-30 | 10 | https://github.com/keshavbhatt/whatsie/issues/149 |
| 147 | CLOSED | Cannot open downloaded file directly | downloads | Cannot open a downloaded file directly from the download panel | 2023-11-25 | 2024-05-25 | 0 | https://github.com/keshavbhatt/whatsie/issues/147 |
| 145 | OPEN | doesnt recieve/send messages | login/session | Background freezes; messages only flow for ~10 s after reload | 2023-11-18 | - | 8 | https://github.com/keshavbhatt/whatsie/issues/145 |
| 144 | OPEN | WhatSie closes itself | crash | Closes itself repeatedly (Kubuntu 22.04) | 2023-11-18 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/144 |
| 143 | CLOSED | Unread messages count in systray noification icon not working well  | tray/window | Unread badge shows 1-2 max and ignores pre-existing unread on start | 2023-11-16 | 2024-10-19 | 2 | https://github.com/keshavbhatt/whatsie/issues/143 |
| 142 | OPEN | Whatsie eats up available ram during syncing. | performance | Exhausts all RAM during sync | 2023-11-13 | - | 4 | https://github.com/keshavbhatt/whatsie/issues/142 |
| 141 | CLOSED | Native notification doesn't automatically discard after viewing message | notifications | Native notifications never auto-dismissed after reading | 2023-11-12 | 2024-10-19 | 2 | https://github.com/keshavbhatt/whatsie/issues/141 |
| 140 | CLOSED | Is it possible to group  chats ? | feature-request | Group chats into custom folders (wontfix) | 2023-11-11 | 2024-05-25 | 1 | https://github.com/keshavbhatt/whatsie/issues/140 |
| 139 | OPEN | Quickly mute/unmute? | feature-request | Quick mute/unmute toggle in tray menu | 2023-11-03 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/139 |
| 138 | CLOSED | Info: Building under Debian testing KDE Plasma 5  | docs/question | Build notes for Debian testing/KDE | 2023-11-01 | 2023-11-01 | 4 | https://github.com/keshavbhatt/whatsie/issues/138 |
| 137 | OPEN | Fedora Silverblue: No Files/Pictures/Attachments from Desktop accessible | packaging/distro | Fedora Silverblue: no access to Desktop files/attachments | 2023-10-11 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/137 |
| 136 | OPEN | Add press up arrow to edit last message | feature-request | Up-arrow to edit last message | 2023-09-24 | - | 0 | https://github.com/keshavbhatt/whatsie/issues/136 |
| 135 | OPEN | Keep window position | tray/window | Remember window position after restore from tray | 2023-09-12 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/135 |
| 134 | CLOSED | CALLS and ADDING STATUS | feature-request | Calls and posting status | 2023-09-11 | 2024-05-25 | 2 | https://github.com/keshavbhatt/whatsie/issues/134 |
| 133 | CLOSED | Bug: Unable to send or receive messages | login/session | Cannot send or receive after building from source | 2023-09-09 | 2024-05-25 | 2 | https://github.com/keshavbhatt/whatsie/issues/133 |
| 132 | OPEN | Multiple languages spell checker | spellcheck/i18n | Multiple spell-check languages simultaneously | 2023-08-26 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/132 |
| 131 | OPEN | Whatsie takes too much resources (CPU/bandwidth) | performance | Too much CPU/bandwidth | 2023-08-22 | - | 11 | https://github.com/keshavbhatt/whatsie/issues/131 |
| 130 | CLOSED | Unable to build | packaging/distro | Cannot build on Fedora 38: qmake picks Qt6, no webengine module | 2023-08-19 | 2023-08-20 | 4 | https://github.com/keshavbhatt/whatsie/issues/130 |
| 129 | CLOSED | Errors during make | packaging/distro | Build errors: QWebEngineDownloadItem::downloadFileName missing (old Qt) | 2023-08-02 | 2023-08-08 | 7 | https://github.com/keshavbhatt/whatsie/issues/129 |
| 128 | OPEN | Monochromatic tray icon | tray/window | Monochromatic tray icon | 2023-07-07 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/128 |
| 127 | CLOSED | Hide tray icon | tray/window | Setting to hide tray icon | 2023-07-04 | 2024-04-09 | 3 | https://github.com/keshavbhatt/whatsie/issues/127 |
| 126 | CLOSED | Not able to send messages on Ubuntu 23.04, after reboot old messages are remarked as unread | login/session | Cannot send on Ubuntu 23.04; read messages re-marked unread after reboot | 2023-06-28 | 2024-05-25 | 11 | https://github.com/keshavbhatt/whatsie/issues/126 |
| 125 | OPEN | Feature Request: Support Windows  | feature-request | Windows support | 2023-06-27 | - | 8 | https://github.com/keshavbhatt/whatsie/issues/125 |
| 124 | OPEN | Zoom factor scale values | theme/ui | Allow arbitrary zoom factor values per monitor | 2023-06-14 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/124 |
| 123 | CLOSED | Pacstall package manager | packaging/distro | Pacstall package | 2023-06-11 | 2023-07-04 | 3 | https://github.com/keshavbhatt/whatsie/issues/123 |
| 122 | OPEN | Add unread notification indicator to task bar | feature-request | Unread badge on taskbar/dock (libunity) | 2023-06-06 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/122 |
| 121 | CLOSED | Clicking a native notification doesn't take me to the message | notifications | Clicking a native notification does not open the relevant chat | 2023-06-06 | 2026-03-29 | 2 | https://github.com/keshavbhatt/whatsie/issues/121 |
| 120 | OPEN | no notification sound | notifications | No notification sound (Arch KDE) | 2023-06-04 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/120 |
| 119 | OPEN | Build on macos? | packaging/distro | Build on macOS? | 2023-06-01 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/119 |
| 118 | OPEN | Whatsie settings accessible even when app is locked | security/privacy | Settings (with password) reachable from tray menu while app is locked | 2023-05-28 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/118 |
| 117 | OPEN | Whatsie Loading Visual Bug | theme/ui | Loading animation rectangle visible in dark mode | 2023-05-20 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/117 |
| 116 | CLOSED | Whatsie not showing number of unread messages on dock and tray icon | tray/window | Unread count missing on tray/dock (Flatpak, Pop!_OS) | 2023-05-19 | 2023-05-20 | 0 | https://github.com/keshavbhatt/whatsie/issues/116 |
| 115 | CLOSED | "Read more" link at end of message freezes Whatsie making it eat all CPU time | performance | "Read more" + "Message info" freezes app in CPU loop | 2023-05-17 | 2023-05-19 | 6 | https://github.com/keshavbhatt/whatsie/issues/115 |
| 114 | OPEN | Whatsie abruptly stops after waking up computer, needing to restart | webengine/whatsapp-breakage | After wake: loadingQuirk() gives up, resets UA and quits | 2023-05-13 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/114 |
| 113 | CLOSED | Not able to send messages with latest SNAP update on Ubuntu 22.04 | login/session | Cannot send after snap update; QtWebEngineProcess pins a core | 2023-05-11 | 2023-05-18 | 7 | https://github.com/keshavbhatt/whatsie/issues/113 |
| 112 | OPEN | When will we be able to make audio and video call by whatsapp for linux? | feature-request | Audio/video calls (wontfix, upstream) | 2023-04-08 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/112 |
| 111 | OPEN | Can't Upload Any Attachments in Whatsie (Ubuntu 22.04) | bug | Cannot upload attachments: drag-drop fails, file dialog only shows "/" (snap) | 2023-03-30 | - | 8 | https://github.com/keshavbhatt/whatsie/issues/111 |
| 110 | CLOSED | Downloading more than one image results in replacement | downloads | Downloading several images overwrites with same filename | 2023-03-15 | 2023-03-22 | 3 | https://github.com/keshavbhatt/whatsie/issues/110 |
| 109 | OPEN | WebEngineContext error while starting Whatsie | bug | GPU: WebEngineContext/OpenGL dummy-context failure at start (snap) | 2023-03-10 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/109 |
| 108 | OPEN | Use a different icon when Whatsie is not connected / not logged in | tray/window | Different tray icon when disconnected/not logged in | 2023-03-07 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/108 |
| 107 | OPEN | Whatsie stops updating randomly | login/session | Stops receiving/sending randomly; segfault in libQt5Core in syslog | 2023-03-06 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/107 |
| 106 | CLOSED | Add video calling | feature-request | Video calling | 2023-03-04 | 2023-04-04 | 4 | https://github.com/keshavbhatt/whatsie/issues/106 |
| 105 | OPEN | The window was totally disfigured after I switched from wayland to gdm3 on Ubuntu 22.04. | bug | GPU: window garbled after switching Wayland->X11; no HW-accel toggle | 2023-02-23 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/105 |
| 104 | OPEN | Whatsie and QtWebEngineProc have huge CPU requirements | performance | Whatsie + QtWebEngineProcess huge CPU | 2023-02-15 | - | 4 | https://github.com/keshavbhatt/whatsie/issues/104 |
| 103 | OPEN | [User Experience Issue] Application Always Opening At The "Main Monitor" \| Only When First Time Startup | tray/window | Always opens on primary monitor; remember monitor | 2023-02-13 | - | 2 | https://github.com/keshavbhatt/whatsie/issues/103 |
| 102 | CLOSED | Render Process Crash | crash | "Render process exited with code 134" ~6x/day (Arch) | 2023-02-10 | 2023-07-04 | 2 | https://github.com/keshavbhatt/whatsie/issues/102 |
| 101 | CLOSED | Random segmentation faults | crash | Random segfaults | 2023-02-06 | 2023-07-04 | 2 | https://github.com/keshavbhatt/whatsie/issues/101 |
| 99 | CLOSED | Whatsie gets unlinked when marking message as unread | login/session | Device unlinked ("logged out due to unexpected issue") after marking unread | 2023-01-23 | 2023-01-26 | 2 | https://github.com/keshavbhatt/whatsie/issues/99 |
| 98 | OPEN | how do i translate this to other languages? | spellcheck/i18n | How to translate the app | 2023-01-22 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/98 |
| 97 | CLOSED | Doesn't find other partition on my SSD Drive | packaging/distro | Snap cannot see other partitions (removable-media plug) | 2023-01-11 | 2023-01-26 | 6 | https://github.com/keshavbhatt/whatsie/issues/97 |
| 96 | CLOSED | Native notifications | notifications | Use native (KNotification/libnotify) notifications instead of QtWebEngineNotification | 2022-12-23 | 2023-01-16 | 2 | https://github.com/keshavbhatt/whatsie/issues/96 |
| 95 | CLOSED | Are there any plans for a TUI version? | feature-request | TUI/CLI client | 2022-12-20 | 2023-01-26 | 2 | https://github.com/keshavbhatt/whatsie/issues/95 |
| 94 | CLOSED | How scheduler message? | feature-request | Scheduled messages | 2022-11-30 | 2023-01-26 | 2 | https://github.com/keshavbhatt/whatsie/issues/94 |
| 93 | OPEN | Add monocrome tray icons | tray/window | Monochrome tray icons for light/dark panels | 2022-11-30 | - | 6 | https://github.com/keshavbhatt/whatsie/issues/93 |
| 92 | OPEN | cannot send messages anymore | login/session | Cannot send messages anymore (receives fine) | 2022-11-13 | - | 9 | https://github.com/keshavbhatt/whatsie/issues/92 |
| 91 | CLOSED | Missing indicator in flatpak | tray/window | Indicator missing in Flatpak; needs XDG_CURRENT_DESKTOP Unity hint | 2022-11-13 | 2023-07-04 | 2 | https://github.com/keshavbhatt/whatsie/issues/91 |
| 90 | CLOSED | Excessive ram and vram usage | performance | RAM/VRAM doubles each open/close, up to 700 MB | 2022-11-02 | 2022-12-01 | 3 | https://github.com/keshavbhatt/whatsie/issues/90 |
| 89 | CLOSED | [Feature Request] Add some security measures | security/privacy | Add app lock / security measures | 2022-10-31 | 2022-11-05 | 3 | https://github.com/keshavbhatt/whatsie/issues/89 |
| 88 | CLOSED | Not sending or receiving messages | login/session | Not sending or receiving (Ubuntu 22, Mint) | 2022-10-24 | 2024-05-25 | 11 | https://github.com/keshavbhatt/whatsie/issues/88 |
| 87 | CLOSED | Whatsie keeps logging out | login/session | Keeps logging out after successful link | 2022-10-11 | 2024-01-09 | 3 | https://github.com/keshavbhatt/whatsie/issues/87 |
| 86 | CLOSED | Whatsie stuck on "organizing messages" | login/session | Stuck on "organizing messages" | 2022-10-10 | 2024-05-25 | 8 | https://github.com/keshavbhatt/whatsie/issues/86 |
| 85 | CLOSED | Icon missing in AUR install | packaging/distro | Icon missing in AUR install (Wayland) | 2022-10-08 | 2023-01-27 | 0 | https://github.com/keshavbhatt/whatsie/issues/85 |
| 84 | CLOSED | It looks as I was kicked out of some group chats | webengine/whatsapp-breakage | Shows "no longer a participant" in groups the phone is still in | 2022-10-08 | 2022-12-01 | 1 | https://github.com/keshavbhatt/whatsie/issues/84 |
| 83 | CLOSED | Mobile | feature-request | Mobile-friendly layout (PinePhone) | 2022-10-06 | 2022-12-01 | 1 | https://github.com/keshavbhatt/whatsie/issues/83 |
| 82 | CLOSED | Group Icons not appreading in flatpack version | notifications | Group icons missing in Flatpak notifications | 2022-10-05 | 2024-07-08 | 0 | https://github.com/keshavbhatt/whatsie/issues/82 |
| 81 | CLOSED | Error: "Server's certificate is not trusted." | webengine/whatsapp-breakage | "Server's certificate is not trusted" popup loop (snap) | 2022-09-24 | 2023-01-26 | 5 | https://github.com/keshavbhatt/whatsie/issues/81 |
| 80 | CLOSED | Whatsie stuck on "organizing messages" | login/session | Stuck on "organizing messages", browser works | 2022-09-23 | 2023-01-26 | 2 | https://github.com/keshavbhatt/whatsie/issues/80 |
| 79 | CLOSED | Spell checker doesn't work | spellcheck/i18n | Spell checker does not flag errors (Spanish) | 2022-09-22 | 2024-10-09 | 6 | https://github.com/keshavbhatt/whatsie/issues/79 |
| 78 | CLOSED | Dictionary | spellcheck/i18n | No dictionary available (Arch) | 2022-09-22 | 2024-10-09 | 3 | https://github.com/keshavbhatt/whatsie/issues/78 |
| 77 | CLOSED | Support for Flatpak and Appimage | packaging/distro | Flatpak and AppImage support | 2022-09-20 | 2022-10-07 | 2 | https://github.com/keshavbhatt/whatsie/issues/77 |
| 76 | OPEN | Feature request : add prefered font size to settings... | theme/ui | Preferred font size setting for low-vision users | 2022-09-13 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/76 |
| 75 | OPEN | Looks like the webapp doesn't work when connected to vpn | feature-request | Proxy support / honour system proxy (VPN breaks it) | 2022-09-12 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/75 |
| 74 | CLOSED | nix package - whatsie | packaging/distro | Nix package | 2022-09-10 | 2025-03-13 | 0 | https://github.com/keshavbhatt/whatsie/issues/74 |
| 73 | CLOSED | Whatsie closes immediately  | crash | Closes immediately after sending a document | 2022-09-02 | 2022-09-03 | 2 | https://github.com/keshavbhatt/whatsie/issues/73 |
| 72 | CLOSED | CPU usage is high | performance | High CPU | 2022-08-31 | 2022-09-03 | 3 | https://github.com/keshavbhatt/whatsie/issues/72 |
| 71 | CLOSED | Memory Leak | performance | Memory leak: 10 GB RAM (Fedora snap) | 2022-08-30 | 2022-09-03 | 8 | https://github.com/keshavbhatt/whatsie/issues/71 |
| 70 | CLOSED | qmake error >> Project ERROR: Could not find feature webengine-spellchecker. | packaging/distro | qmake: webengine-spellchecker feature missing | 2022-08-24 | 2022-08-27 | 10 | https://github.com/keshavbhatt/whatsie/issues/70 |
| 69 | CLOSED | Blurry status icon  | tray/window | Blurry tray icon at 200% scaling | 2022-08-07 | 2022-09-13 | 7 | https://github.com/keshavbhatt/whatsie/issues/69 |
| 68 | CLOSED | INFO snap "whatsie" has bad plugs or slots: audio-playback (unknown interface "audio-playback"); audio-record (unknown interface "audio-record") | packaging/distro | Snap bad plugs audio-playback/record on Deepin; won't launch | 2022-08-02 | 2022-09-03 | 2 | https://github.com/keshavbhatt/whatsie/issues/68 |
| 67 | CLOSED | Lock-screen is triggered also during normal app usage (scroling a chat for example) | security/privacy | Lock screen triggers during active use (idle detection ignores web input) | 2022-08-01 | 2022-08-27 | 2 | https://github.com/keshavbhatt/whatsie/issues/67 |
| 66 | OPEN | Whatsie preventing system shutdown | tray/window | Prevents KDE shutdown ("exit prevented") - closeEvent veto | 2022-07-22 | - | 4 | https://github.com/keshavbhatt/whatsie/issues/66 |
| 64 | OPEN | [feature request] toggle function via command line | feature-request | CLI toggle command to raise/focus or hide window | 2022-07-15 | - | 3 | https://github.com/keshavbhatt/whatsie/issues/64 |
| 63 | OPEN | Problems with character cedilha (ç) in US International keyboard layout | spellcheck/i18n | Cedilla (ç) dead key wrong in snap | 2022-07-09 | - | 4 | https://github.com/keshavbhatt/whatsie/issues/63 |
| 62 | CLOSED | Theme switching | theme/ui | Theme selection not remembered | 2022-07-08 | 2022-07-22 | 2 | https://github.com/keshavbhatt/whatsie/issues/62 |
| 61 | OPEN | build only needed dictionaries | packaging/distro | Build only needed dictionaries | 2022-07-08 | - | 1 | https://github.com/keshavbhatt/whatsie/issues/61 |
| 60 | CLOSED | versions mismatch | packaging/distro | Version string mismatch between .pro and VERSION.txt | 2022-07-08 | 2022-09-03 | 1 | https://github.com/keshavbhatt/whatsie/issues/60 |
| 58 | OPEN | Stuck at "organizing Messages" Screen | login/session | Stuck at "Organizing Messages" | 2022-06-28 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/58 |
| 57 | CLOSED | build instructions? | docs/question | Build instructions / dependencies | 2022-06-26 | 2022-07-22 | 3 | https://github.com/keshavbhatt/whatsie/issues/57 |
| 54 | CLOSED | Build failure: error: 'GIT_BRANCH' was not declared in this scope | packaging/distro | Build fails: GIT_BRANCH not declared | 2022-06-24 | 2022-06-24 | 3 | https://github.com/keshavbhatt/whatsie/issues/54 |
| 51 | CLOSED | High cpu usage | performance | ~30% CPU after opening (snap) | 2022-06-23 | 2022-06-24 | 12 | https://github.com/keshavbhatt/whatsie/issues/51 |
| 50 | OPEN | unneeded depends at build time | packaging/distro | Unneeded link-time dependencies | 2022-06-23 | - | 5 | https://github.com/keshavbhatt/whatsie/issues/50 |
| 49 | CLOSED | git discovering should be disabled in a release | packaging/distro | git discovery breaks tarball builds | 2022-06-22 | 2022-06-24 | 3 | https://github.com/keshavbhatt/whatsie/issues/49 |
| 48 | OPEN | I have to log out and re-link my device each time | login/session | Must logout/relink each start or stuck at "organizing messages" | 2022-06-20 | - | 7 | https://github.com/keshavbhatt/whatsie/issues/48 |
| 47 | CLOSED | Right click on Trayicon minimize window | tray/window | Right-click on tray icon hides window instead of showing menu | 2022-06-17 | 2022-07-22 | 3 | https://github.com/keshavbhatt/whatsie/issues/47 |
| 46 | CLOSED | On maximized window, useful content doesn't expand on full viewport | theme/ui | Maximized: content does not expand to full viewport | 2022-06-17 | 2022-06-23 | 2 | https://github.com/keshavbhatt/whatsie/issues/46 |
| 44 | CLOSED | Change keyboard shortcuts ? Build project ? | feature-request | Configurable keyboard shortcuts (Ctrl+W conflict) | 2022-06-16 | 2022-06-16 | 2 | https://github.com/keshavbhatt/whatsie/issues/44 |
| 43 | CLOSED | Answer/make call and video call | feature-request | Make/answer calls | 2022-06-09 | 2022-06-16 | 3 | https://github.com/keshavbhatt/whatsie/issues/43 |
| 42 | CLOSED | Opens when click on any notification | notifications | Opens when clicking ANY program's notification (11 comments) | 2022-05-29 | 2026-03-29 | 11 | https://github.com/keshavbhatt/whatsie/issues/42 |
| 41 | CLOSED | Custom notification shown on secondary screen | notifications | Custom notification shown on secondary monitor | 2022-05-27 | 2022-06-23 | 6 | https://github.com/keshavbhatt/whatsie/issues/41 |
| 40 | CLOSED | build system lacks install steps | packaging/distro | Build system lacks install steps for icons; undeclared deps | 2022-05-16 | 2022-07-22 | 8 | https://github.com/keshavbhatt/whatsie/issues/40 |
| 39 | CLOSED | Whatsie consuming 7-10% CPU on idle, even if minimized to tray | performance | 7-10% CPU idle in tray | 2022-05-12 | 2022-05-12 | 2 | https://github.com/keshavbhatt/whatsie/issues/39 |
| 38 | CLOSED | "An Instance Of Whatsie Is Already Running" | tray/window | "Instance already running" warning instead of raising window | 2022-05-04 | 2022-05-10 | 10 | https://github.com/keshavbhatt/whatsie/issues/38 |
| 37 | CLOSED | At start, Whatsie seems to have a minimum width that I cannot control | tray/window | Enforced minimum width at start | 2022-04-20 | 2022-05-07 | 4 | https://github.com/keshavbhatt/whatsie/issues/37 |
| 36 | CLOSED | Every time I link my device it always auto-logout. | login/session | Auto-logout every time after linking | 2022-04-19 | 2022-06-24 | 12 | https://github.com/keshavbhatt/whatsie/issues/36 |
| 34 | CLOSED | [Bug / Error] "Your device could not link due to a sync error" | login/session | "Could not link due to a sync error"; logs out all web devices | 2022-03-29 | 2022-03-30 | 5 | https://github.com/keshavbhatt/whatsie/issues/34 |
| 33 | CLOSED | Feature Requests for Whatsie  | feature-request | Paste image from clipboard; auto link embeds | 2022-03-26 | 2022-03-30 | 2 | https://github.com/keshavbhatt/whatsie/issues/33 |
| 31 | CLOSED | Downloading file is creating folder of name passed | downloads | Download creates a folder with the passed name | 2022-03-22 | 2022-03-22 | 0 | https://github.com/keshavbhatt/whatsie/issues/31 |
| 30 | CLOSED | Feature Requests for Whatsie | feature-request | Toggle via shortcut; download behaviour | 2022-03-20 | 2022-05-07 | 8 | https://github.com/keshavbhatt/whatsie/issues/30 |
| 29 | CLOSED | How to change passwd after set one | security/privacy | No way to change lock password once set | 2022-03-07 | 2022-03-30 | 3 | https://github.com/keshavbhatt/whatsie/issues/29 |
| 28 | CLOSED | Unable to use enter for new line | feature-request | Enter for newline, Ctrl+Enter to send | 2022-03-03 | 2022-03-09 | 2 | https://github.com/keshavbhatt/whatsie/issues/28 |
| 27 | CLOSED | .. | packaging/distro | snap install network error | 2022-02-27 | 2022-02-27 | 0 | https://github.com/keshavbhatt/whatsie/issues/27 |
| 26 | CLOSED | Gnome Shell native desktop notification (request) | notifications | GNOME Shell native notification option | 2022-01-27 | 2022-03-30 | 8 | https://github.com/keshavbhatt/whatsie/issues/26 |
| 25 | CLOSED | Whatsie started to crash on Fedora 35 | crash | Crashes on open on Fedora 35 (snap) | 2022-01-21 | 2022-02-20 | 6 | https://github.com/keshavbhatt/whatsie/issues/25 |
| 24 | CLOSED | Theme keeps changing back to Light mode | theme/ui | Theme reverts to light on reboot | 2022-01-04 | 2022-02-21 | 3 | https://github.com/keshavbhatt/whatsie/issues/24 |
| 23 | CLOSED | Checking 'Use Native File Dialog' causes File Dialog not to open in Ubuntu 21.10 | bug | "Use Native File Dialog" makes dialog never open (snap) | 2021-12-09 | 2022-02-20 | 3 | https://github.com/keshavbhatt/whatsie/issues/23 |
| 22 | CLOSED | Failed to load on Debian 11 - X11 | crash | Fails to load on Debian 11 X11 (snap) | 2021-10-17 | 2022-02-21 | 1 | https://github.com/keshavbhatt/whatsie/issues/22 |
| 21 | CLOSED | portuguese accents | spellcheck/i18n | Portuguese accents/dead keys broken (39 comments) | 2021-09-24 | 2022-02-20 | 39 | https://github.com/keshavbhatt/whatsie/issues/21 |
| 20 | CLOSED | Tray icon menu issue | tray/window | Tray menu flashes and minimizes app (Zorin) | 2021-09-23 | 2022-03-30 | 4 | https://github.com/keshavbhatt/whatsie/issues/20 |
| 19 | CLOSED | Start minimized (request) | feature-request | Start minimized option | 2021-09-18 | 2022-03-20 | 7 | https://github.com/keshavbhatt/whatsie/issues/19 |
| 18 | CLOSED | crash on Xubunt 21.04 (possibly after JetBrainz Mono font installed) | crash | Segfault on Xubuntu after font install | 2021-08-23 | 2022-02-19 | 9 | https://github.com/keshavbhatt/whatsie/issues/18 |
| 17 | CLOSED | Fcitx input method support | spellcheck/i18n | Fcitx input method support | 2021-08-19 | 2022-02-20 | 6 | https://github.com/keshavbhatt/whatsie/issues/17 |
| 16 | CLOSED | New messages and tray icon | tray/window | Unread badge on tray icon | 2021-08-19 | 2022-03-30 | 6 | https://github.com/keshavbhatt/whatsie/issues/16 |
| 15 | CLOSED | iBus [Keyman] Integration | spellcheck/i18n | iBus/Keyman layouts do not type | 2021-07-27 | 2022-02-20 | 9 | https://github.com/keshavbhatt/whatsie/issues/15 |
| 14 | CLOSED | Implement IPC | feature-request | IPC: raise existing instance instead of error | 2021-07-24 | 2022-05-10 | 1 | https://github.com/keshavbhatt/whatsie/issues/14 |
| 12 | CLOSED | Stupid question | docs/question | How to open settings | 2021-07-18 | 2021-07-18 | 1 | https://github.com/keshavbhatt/whatsie/issues/12 |
| 11 | CLOSED | Disable "Application is minimized to system tray" notification | notifications | Disable "minimized to tray" notification | 2021-07-16 | 2021-07-18 | 1 | https://github.com/keshavbhatt/whatsie/issues/11 |
| 10 | CLOSED | Ctrl + Arrow word marking is only left-to-right, "Whatsie is already running" | spellcheck/i18n | RTL Ctrl+Arrow word jumps reversed; raise running instance | 2021-07-14 | 2022-05-07 | 2 | https://github.com/keshavbhatt/whatsie/issues/10 |
| 9 | CLOSED | Window minimum width is too small | tray/window | Minimum width too small | 2021-07-11 | 2021-07-18 | 5 | https://github.com/keshavbhatt/whatsie/issues/9 |
| 8 | CLOSED | Magnification | theme/ui | Zoom in/out | 2021-06-29 | 2021-06-29 | 1 | https://github.com/keshavbhatt/whatsie/issues/8 |
| 7 | CLOSED | accentuation is not working | spellcheck/i18n | Accented characters not accepted (58 comments) | 2021-06-09 | 2022-02-20 | 58 | https://github.com/keshavbhatt/whatsie/issues/7 |
| 6 | CLOSED | Missing indicator in Ubuntu Mate | tray/window | AppIndicator missing on Ubuntu MATE snap | 2021-06-07 | 2021-06-09 | 8 | https://github.com/keshavbhatt/whatsie/issues/6 |
| 5 | CLOSED | Open other notifications make whatsie active too | notifications | Clicking other apps' notifications activates Whatsie | 2021-05-17 | 2021-06-07 | 6 | https://github.com/keshavbhatt/whatsie/issues/5 |
| 4 | CLOSED | Error when load app on Ubuntu | crash | Error on load (Ubuntu 20.04 snap, GTK modules) | 2021-05-08 | 2021-05-08 | 4 | https://github.com/keshavbhatt/whatsie/issues/4 |
| 3 | CLOSED | How to open to chat new numbers? | feature-request | Open wa.me/send?phone links | 2021-05-07 | 2021-05-30 | 4 | https://github.com/keshavbhatt/whatsie/issues/3 |
| 2 | CLOSED | Native notifications don't work in Kubuntu 20.04.2 | notifications | Native notification vanishes instantly on Kubuntu | 2021-04-29 | 2021-04-30 | 4 | https://github.com/keshavbhatt/whatsie/issues/2 |
| 1 | CLOSED | permission management | other | Permission management (maintainer note) | 2021-04-10 | 2021-04-11 | 1 | https://github.com/keshavbhatt/whatsie/issues/1 |

## All issues: shakaran/whatly (39)

| # | State | Title | Category | Gist | Created | Closed | Comments | URL |
|---|---|---|---|---|---|---|---|---|
| 103 | OPEN | How to open the settings dialog with the flatpak version? | docs/question | How to open the settings dialog in the Flatpak build | 2026-08-27 | - | 0 | https://github.com/shakaran/whatly/issues/103 |
| 102 | CLOSED | [bug] Getting recent messages keep failing when it should not | login/session | "Getting recent messages" keeps failing while Firefox/ZapZap work | 2026-08-25 | 2026-08-25 | 1 | https://github.com/shakaran/whatly/issues/102 |
| 98 | OPEN | [BUG] Application hangs with ~100% CPU usage upon restart after changing settings | bug | App hangs at 100% CPU on restart after changing a restart-required setting | 2026-08-20 | - | 0 | https://github.com/shakaran/whatly/issues/98 |
| 97 | OPEN | Call and Video Call Buttons greyed out ("unsupported browser" message) | media | Call/video buttons greyed out "unsupported browser" (snap 7.3.0, Chrome/140 UA) | 2026-08-20 | - | 2 | https://github.com/shakaran/whatly/issues/97 |
| 96 | OPEN | suggestion: stop whatly from showing the "this image can't be uploaded in HD" for every image (make it stop the first time) | media | "Image can't be uploaded in HD" dialog for every image; show once | 2026-08-19 | - | 1 | https://github.com/shakaran/whatly/issues/96 |
| 94 | CLOSED | how to set an environement table? (e.g make whatly run at monitor refresh rate) | docs/question | How to set env vars (run at monitor refresh rate) | 2026-08-17 | 2026-08-18 | 2 | https://github.com/shakaran/whatly/issues/94 |
| 93 | CLOSED | packaging(windows): no Windows build ships the proprietary codecs, and the notice sends the user after a package that does not exist | packaging/distro | Windows builds ship no proprietary codecs; notice points to non-existent package | 2026-08-16 | 2026-08-17 | 1 | https://github.com/shakaran/whatly/issues/93 |
| 91 | OPEN | follow-up: off-screen unload timer restarts with a full delay for every window (#82) | bug | Off-screen unload timer restarts with full delay for every window (follow-up to #82) | 2026-08-15 | - | 0 | https://github.com/shakaran/whatly/issues/91 |
| 85 | OPEN | proposal(update): the AppImage's zsync delta update is published with every release, and nothing in the app ever invokes it | packaging/distro | AppImage zsync delta update published but never invoked by the app | 2026-08-13 | - | 1 | https://github.com/shakaran/whatly/issues/85 |
| 84 | OPEN | bug: Qt Wayland rendering fails with NVIDIA on Plasma Wayland on .deb (but works when forcing X11) | bug | GPU: Qt Wayland rendering fails with NVIDIA on Plasma (.deb); X11 works | 2026-08-13 | - | 4 | https://github.com/shakaran/whatly/issues/84 |
| 81 | CLOSED | bug: new updates broke the notification logo again on KDE | notifications | Update broke notification logo again on KDE (Flatpak version mismatch) | 2026-08-13 | 2026-08-13 | 1 | https://github.com/shakaran/whatly/issues/81 |
| 76 | CLOSED | proposal(dictionaries): ship the system language only and fetch the rest on demand — 43.2 MB of the install is dictionaries nobody asked for | packaging/distro | Ship only system-language dictionary; fetch others on demand (43 MB) | 2026-08-12 | 2026-08-13 | 1 | https://github.com/shakaran/whatly/issues/76 |
| 74 | CLOSED | bug: clicking the notification on KDE (update prompt) does nothing | notifications | Clicking the update notification on KDE does nothing | 2026-08-12 | 2026-08-13 | 1 | https://github.com/shakaran/whatly/issues/74 |
| 71 | OPEN | proposal(packaging): an update-only Windows download would be 2 MB instead of 143 MB — measured across 7.0.0 → 7.1.0, where 1 of 139 files changed | packaging/distro | Update-only Windows download would be 2 MB instead of 143 MB | 2026-08-11 | - | 1 | https://github.com/shakaran/whatly/issues/71 |
| 68 | CLOSED | packaging(windows): the .msi and the portable .zip omit the MSVC runtime that whatly.exe imports, so a machine without the VC++ redistributable cannot launch either | packaging/distro | Windows .msi/.zip omit MSVC runtime; launch fails without VC++ redist | 2026-08-11 | 2026-08-12 | 2 | https://github.com/shakaran/whatly/issues/68 |
| 67 | CLOSED | packaging: the two v7.1.0 rpms collide by release digit, the portable one loses 6 of 7 icons and its AppStream data, and the native one ships 54 MB of DWARF | packaging/distro | Two v7.1.0 RPMs collide by release digit; portable one loses icons; native ships 54 MB DWARF | 2026-08-11 | 2026-08-12 | 1 | https://github.com/shakaran/whatly/issues/67 |
| 53 | CLOSED | Can't send or view videos | media | Cannot send or view videos (codecs) on Debian 14 AppImage/DEB | 2026-08-06 | 2026-08-11 | 5 | https://github.com/shakaran/whatly/issues/53 |
| 46 | CLOSED | Dictionaries as downloads: a settings list with a Download/Delete button per language (45 MB of every install) | spellcheck/i18n | Dictionaries as on-demand downloads with per-language Download/Delete (45 MB) | 2026-08-04 | 2026-08-11 | 5 | https://github.com/shakaran/whatly/issues/46 |
| 43 | OPEN | Unable to log in using phone number instead of QR. | login/session | Cannot log in with phone number (works in WhatSie), 12 comments | 2026-08-03 | - | 12 | https://github.com/shakaran/whatly/issues/43 |
| 42 | CLOSED | Security: App Lock passcode stored as reversible Base64, not hashed (unverified — static review only) | security/privacy | App lock passcode stored as reversible Base64, not hashed | 2026-08-03 | 2026-08-03 | 1 | https://github.com/shakaran/whatly/issues/42 |
| 41 | CLOSED | Security: App Lock appears bypassable via Quick Compose and notification reply (unverified — static review only) | security/privacy | App lock bypassable via Quick Compose and notification inline reply | 2026-08-03 | 2026-08-03 | 2 | https://github.com/shakaran/whatly/issues/41 |
| 38 | CLOSED | Bug: notifications on KDE don't have Whatly logo (unknown logo instead) | notifications | KDE notifications show unknown logo instead of Whatly logo (Flatpak + libnotify) | 2026-08-02 | 2026-08-03 | 1 | https://github.com/shakaran/whatly/issues/38 |
| 37 | CLOSED | Strange font rendering in dark mode on Linux | theme/ui | Strange font hinting/antialiasing in dark mode | 2026-07-30 | 2026-08-01 | 4 | https://github.com/shakaran/whatly/issues/37 |
| 36 | CLOSED | Bug: smooth scroll doesn't work even when enabled | bug | Smooth scrolling has no effect even when enabled | 2026-07-30 | 2026-08-03 | 6 | https://github.com/shakaran/whatly/issues/36 |
| 35 | CLOSED | Bug: missing theme color when hovering over message dropdown | theme/ui | Missing theme colour on hover of message dropdown | 2026-07-30 | 2026-08-01 | 3 | https://github.com/shakaran/whatly/issues/35 |
| 34 | CLOSED | bug: unable to attach image/video files | media | Cannot attach image/video; HD-quality dialog loops until quit (AppImage 6.6-6.8) | 2026-07-29 | 2026-08-03 | 11 | https://github.com/shakaran/whatly/issues/34 |
| 33 | CLOSED | Whatly and Phone Calls | docs/question | Does Whatly support phone calls? | 2026-07-29 | 2026-07-30 | 2 | https://github.com/shakaran/whatly/issues/33 |
| 32 | CLOSED | Drag-and-drop attach silently does nothing in the Flatpak (any file outside ~/Downloads) | bug | Drag-and-drop attach silently fails in Flatpak for files outside ~/Downloads (raw host path) | 2026-07-28 | 2026-08-11 | 4 | https://github.com/shakaran/whatly/issues/32 |
| 30 | CLOSED | Release on AUR | packaging/distro | Release on AUR | 2026-07-27 | 2026-07-28 | 2 | https://github.com/shakaran/whatly/issues/30 |
| 28 | CLOSED | Render process exited with code: 15 | crash | "Render process exited with code 15" endless loop (Flatpak) | 2026-07-27 | 2026-08-03 | 2 | https://github.com/shakaran/whatly/issues/28 |
| 22 | CLOSED | New Window / Dual Accounts | docs/question | Cannot open a second window/tab for a second account (AppImage) | 2026-07-26 | 2026-07-27 | 2 | https://github.com/shakaran/whatly/issues/22 |
| 21 | CLOSED | Pls. Register AppImage on AppImageManager am | packaging/distro | Register AppImage with AM AppImage manager for auto-update | 2026-07-26 | 2026-07-27 | 2 | https://github.com/shakaran/whatly/issues/21 |
| 20 | OPEN | Release on Flathub | packaging/distro | Release on Flathub | 2026-07-26 | - | 1 | https://github.com/shakaran/whatly/issues/20 |
| 15 | CLOSED | High memory usage (possible bug?) | performance | 1.5 GB idle memory vs ~500 MB for other clients (QtWebEngineProcess) | 2026-07-26 | 2026-07-26 | 3 | https://github.com/shakaran/whatly/issues/15 |
| 14 | CLOSED | bug: Monochrome tray icon not working (Debian 14) | tray/window | Monochrome tray icon option has no effect (Debian 14) | 2026-07-26 | 2026-07-28 | 10 | https://github.com/shakaran/whatly/issues/14 |
| 12 | CLOSED | bug: can't install/launch under Debian 14 | packaging/distro | Cannot launch on Debian 14: bundled libnssutil3 too old for system libsoftokn3 | 2026-07-25 | 2026-07-26 | 3 | https://github.com/shakaran/whatly/issues/12 |
| 11 | CLOSED | bug: cannot launch on PikaOS (crash) | crash | AppImage crashes on launch on PikaOS (Wayland plugin missing, portal app ID) | 2026-07-25 | 2026-07-26 | 3 | https://github.com/shakaran/whatly/issues/11 |
| 3 | CLOSED | bug: rpm release force close & core dump on f44 | crash | RPM force-closes with SIGTRAP core dump on Fedora 44 | 2026-07-23 | 2026-07-30 | 3 | https://github.com/shakaran/whatly/issues/3 |
| 2 | CLOSED | Icons on native notifications are blurry (low resolution) | notifications | Native notification icons blurry (low resolution) | 2026-07-18 | 2026-07-18 | 1 | https://github.com/shakaran/whatly/issues/2 |


## Recurring themes (both repos)

Counts are issues that belong primarily to the theme; some issues are cited under two themes. "W#" = whatsie, "Y#" = whatly.

### 1. Session sync stuck / can receive but cannot send (~30 issues)
W#34, 36, 48, 58, 80, 86, 87, 88, 92, 99, 107, 113, 126, 133, 145, 153, 162, 170, 181, 184, 196, 208, 220, 229, 243, 249, 256, 270, 280; Y#43, 102.
The single largest cluster. Symptoms: "organizing messages" / "loading your chats 100%" forever, messages stuck with a clock icon, device silently unlinked, only logout-and-relink helps, breaks again after 1-2 weeks, breaks when the phone app or a second PC is used (W#153, 181), no reconnect after sleep (W#208). Users cannot get logs, so every thread is 5-20 comments of guessing.
**Lessons:** the app has no diagnostics for the web layer (no console/log capture, no "which WA Web build am I on"; whatly PR #79 added exactly that), no visible connection state (W#108 asks for a disconnected tray icon), no network-wait/auto-reload after resume, and a "loadingQuirk" watchdog that silently resets the UA and *quits* the process (W#114). Old Chromium + hard-coded UA makes WhatsApp serve a degraded/unsupported path, which then looks like a sync bug. Need: configurable/auto-updated UA, a reload/relink action in the tray menu, exportable logs, and an explicit "not connected" state.

### 2. WhatsApp Web breaks the wrapper (old Chromium / user-agent / cert dialogs) (19 issues)
W#81, 84, 114, 149, 151, 195, 201, 236, 238, 261, 279, 283, 293, 303, 306, 313, 315, 318; Y#97.
Blank white window after login (W#149/151 in Nov 2023, W#283 in 2026), square avatars and missing rounded corners because the Qt5 WebEngine Chromium (87) lacks CSS features (W#279, 293), "your version of WhatsApp Web doesn't support it" (W#313), a Facebook login page instead of WhatsApp (W#315), HTTP 429 (W#306), calls greyed out as "unsupported browser" (Y#97), and a modal "Server's certificate is not trusted" popup that re-appears in a loop (W#81, 195, 261, 303).
**Lessons:** the UA is hard-coded (W#236 asked how to change it) and Qt5 WebEngine was frozen for two years (W#168 asked for Qt6 in 2024-03, merged 2026-07). Design needs: UA editable in settings with a sane auto-updated default, a Chromium version check that warns instead of breaking, certificate errors handled once (not a modal loop), and a quick "clear persistent data" path that does not nuke everything.

### 3. Crashes: segfaults, render-process death, silent exit from tray (~30 issues)
W#4, 18, 22, 25, 73, 101, 102, 144, 152, 155, 167, 174, 175, 207, 210, 211, 212, 225, 234, 244, 245, 248, 253, 272, 291; Y#3, 11, 28.
Notable: W#212 is a SIGSEGV in `QWidget::setMinimumWidth` from the notification-presenter lambda; W#207/210/211 are "app closes itself when a message arrives while minimized" - i.e. the custom notification widget crashed the process. W#167 froze the whole system. W#225 asks for auto-restart because it "quietly" dies in the tray. W#102/Y#28 are the Chromium render process dying in an endless reload dialog loop.
**Lessons:** UI work done from WebEngine callbacks without guarding widget lifetime; no crash handler, no log file (whatly PR #62 "keep the log running across a restart" and PR #44 startup stamp address this); the "render process exited, reload?" dialog must back off instead of looping.

### 4. GPU / Wayland rendering: blank screens and flicker (11 issues)
W#105, 109, 154, 189, 200, 227, 234, 252, 268; Y#84; plus W#191 (disable GPU option).
EGL/shared-context failures on NVIDIA and in Flatpak give a blank content area; window flicker under Wayland Flatpak (W#227); KWin crashes on fullscreen video (W#268). whatsie shipped `--disable-gpu` for years, which fixed blank screens but blanked video calls (whatsie PR #334 removed it in 2026-08).
**Lessons:** one global flag cannot serve every GPU/compositor; need a user-visible hardware-acceleration toggle (W#105, 191), a safe-mode fallback when the GPU process fails to initialise, native Wayland instead of XWayland (W#189), and a way to pass `QTWEBENGINE_CHROMIUM_FLAGS` from settings (W#203 shows the CLI rejects unknown Chromium flags).

### 5. Memory and CPU consumption (17 issues)
W#39, 51, 71, 72, 90, 104, 115, 131, 142, 163, 185, 190, 194, 241, 255, 269; Y#15.
Reports of 5-12 GB RSS (W#71, 194, 255), V8 heap exhausted at ~3.8 GB then SIGABRT (W#185, 269), RAM doubling on each hide/show (W#90), 7-30% CPU idle in tray (W#39, 51, 190), CPU pegged during sync (W#196, 220). whatly at 1.5 GB idle is 3x other clients (Y#15).
**Lessons:** a permanently loaded WA Web page plus injected JS and a never-cleared render cache; W#90 suggests the page/profile was re-created without releasing. Need: unload/suspend hidden accounts (whatly PR #48, #82 do this), a memory watchdog that reloads the page before V8 dies, an idle-CPU audit of injected polling scripts, and exposing "disable GPU / reduce cache" knobs.

### 6. Notifications (30 issues)
Sub-clusters:
- **App raises when ANY notification is clicked** - W#5, 42 (11 comments), 271, 278. The D-Bus `ActionInvoked` signal was handled without matching the notification id. Recurred from 2021 to 2025.
- **Native notifications never pop up on GNOME/Ubuntu, custom ones don't reach the notification centre** - W#2, 26, 96, 161, 198, 284, 329. QtWebEngine's notification presenter is not a real freedesktop notification; whatsie PR #295 moved to `org.freedesktop.Notifications` directly in 2026.
- **Icons wrong**: colours inverted (W#312, 328 - BGRA vs RGBA when encoding the avatar for D-Bus), blurry (Y#2), missing/unknown logo on KDE (W#82, Y#38, 81), group icons missing in Flatpak (W#82).
- **Nag popups**: "started minimized in tray" shown on every launch - W#11, 205, 316, and three still-open whatsie PRs (#266, 308, 309) to remove it.
- **Behaviour**: clicking does not open the chat (W#121, Y#74), never auto-dismissed (W#141), no sound (W#120), preview toggle removed (W#180), custom popup on wrong monitor (W#41), WhatsApp's own "notifications are off" permission banner has no UI to grant it (W#307).
**Lessons:** implement one notification backend on the freedesktop spec with proper id tracking and correct image byte order; honour DE conventions; do not ship informational popups without a "don't show again"; wire the web `Notification` permission to a real settings toggle.

### 7. Tray icon / indicator differences across desktops (19 issues)
W#6, 16, 20, 47, 69, 85, 91, 93, 108, 116, 127, 128, 143, 173, 193, 274, 292; Y#14.
Indicator missing on MATE/Flatpak unless `XDG_CURRENT_DESKTOP` includes Unity (W#6, 91), right-click hides the window instead of showing the menu (W#20, 47 - fixed by PR #65), left-click does nothing on Plasma Flatpak (W#274), "Restore" greyed out on Wayland (W#292, open PR #335), unread badge wrong/missing (W#16, 116, 143; whatly PR #65/#66/#80 rework it), blurry at 200% (W#69), monochrome icon wanted (W#93, 128) or broken (Y#14), hide-tray option (W#127, 193).
**Lessons:** tray behaviour must not be the only way back to the window (whatly PR #13: "never hide the window when there is no tray icon"); provide badge counting from WhatsApp's own store, not the title; ship SVG/monochrome variants; make click semantics configurable.

### 8. Window management (13 issues)
W#9, 37, 46, 66, 103, 135, 164, 169, 192, 227, 235, 305, 333.
Hard-coded minimum size (W#9, 37, 192 - not scaled by zoom factor), cannot leave fullscreen after video (W#169, 333), stuck maximized (W#305), full-width mode broken (W#46, 235), position/monitor not remembered (W#103, 135), and `closeEvent` veto blocks KDE shutdown (W#66).
**Lessons:** persist geometry + screen, honour `fullScreenRequested` exit, never veto session shutdown, and derive minimum size from zoom.

### 9. Theme not persisted / dark mode regressions (11 issues)
W#24 (2022), 62 (2022), 206 (2024), 299 (2026), 317 (2026), 332 (2026), 290, 157, 300; Y#35, 37; open whatsie PR #326.
The "theme reverts to light on restart" bug was reported and re-fixed four separate times across five years.
**Lessons:** the theme is applied by injected JS racing WhatsApp's own `localStorage` theme key and the Qt palette; there is no single source of truth and no regression test. Need: one setting, applied after page load deterministically, plus a "follow system" mode (W#157) and palette-aware widget styling (W#300 dark-mode checkboxes).

### 10. Sandbox packaging (snap/Flatpak) breaks files, D-Bus, and libraries (25 issues)
W#23, 27, 63, 68, 97, 111, 137, 177, 204, 226, 233, 243, 246, 253, 275, 285, 286, 296, 320; Y#11, 12, 32, 67, 68, 93.
Snap AppArmor blocking D-Bus (W#177, 243, 253), `home` plug dropped in v5 so the file picker cannot see $HOME (W#296), no other partitions (W#97), Silverblue/Flatpak cannot see Desktop (W#137), drag-and-drop reads raw host paths inside the sandbox (Y#32), spellcheck broken by snap revision (W#246), fcitx/cedilla IM missing in snap (W#63, 233), library ABI drift for AUR/rpm/deb (W#275 icu, W#320 xcb-cursor, Y#12 nss), Windows packages missing the MSVC runtime (Y#68).
**Lessons:** use XDG portals for file chooser and drag-drop, declare all plugs/permissions and test them per release, bundle IM plugins, and verify packages on a clean VM before release.

### 11. Build system and downstream packaging friction (18 issues)
W#40, 49, 50, 54, 57, 60, 61, 70, 129, 130, 138, 168, 213, 216, 314; Y#46, 67, 76.
git discovery breaks tarball builds (W#49, 54), version string mismatch (W#60), qmake vs Qt6 confusion (W#130, 314 after the CMake move with no README update), hardening flags overriding distro CFLAGS (W#216), 45 MB of bundled dictionaries (W#61, Y#46, 76).
**Lessons:** reproducible builds from tarballs, versions from a single file, documented dependencies, and optional/on-demand dictionaries.

### 12. Spell-check, dictionaries, input methods and accents (15 issues)
W#7 (58 comments), 21 (39 comments), 10, 15, 17, 63, 78, 79, 132, 156, 233, 246, 262; Y#46, 76.
Accents/dead keys not working in the snap were the two most-commented issues in the whole tracker. Dictionaries silently absent (W#78, 156), multi-language spellcheck wanted (W#132 - whatly PR #73/#78 implemented it).
**Lessons:** IME/Qt platform input plugins must be bundled; the dictionary path must be verified at runtime with a visible error; language selection should be multi-select with on-demand download.

### 13. Attachments, downloads and clipboard (18 issues)
W#23, 31, 33, 97, 110, 111, 147, 273, 285, 296, 297, 301, 311, 336; Y#32, 34, 53, 96.
Custom download manager created a folder named after the file (W#31), overwrote same-named images (W#110), lost the download directory setting after v5 (W#273, 301, 297), cannot open the downloaded file (W#147); attach via drag-drop says "document has no content" (W#111, 285); clipboard image paste does nothing (W#33, 311); HD-quality dialog loops until quit (Y#34, 96); video codecs absent (Y#53, 93).
**Lessons:** do not re-implement Chromium's download path unless it is complete; drag-drop must go through the page's own file input; document the download folder in the UI; a modal dialog must never be re-triggered by its own dismissal.

### 14. App lock / security (8 issues)
W#29, 67, 89, 118, 240, 338; Y#41, 42.
Passcode stored as reversible Base64 under a settings key literally named `asdfg` (Y#42); lock bypassable via Quick Compose / notification reply because `commandSend()` never checks lock state (Y#41); settings (with password) reachable from the tray while locked (W#118); lock triggers during active use because idle detection ignores web input (W#67); "Unlock to access Settings" with no unlock prompt (W#240, 338); no way to change the password (W#29).
**Lessons:** hash the passcode, enforce lock in the command layer not the UI, and make lock state a first-class mode with its own unlock dialog.

### 15. Multi-account / multi-instance and calls (17 issues)
Multi-account: W#153, 166, 176, 181, 231, 259, 327; Y#22. Calls: W#43, 106, 112, 134, 187, 218, 263, 287; Y#33, 97.
Single-instance guard (SingleApplication) plus a single profile made a second account impossible even with a different $HOME (W#176); whatly's Chrome-style tabs (PR #10) answer this but confused users on how to add one (Y#22). Calls were "wontfix upstream" for years (W#112) until WhatsApp enabled them for a Chrome UA; then the device-permission popup did not appear (W#287) and `--disable-gpu` blanked the video (PR #334); whatly's UA still trips "unsupported browser" (Y#97).
**Lessons:** design for N profiles from the start; UA and media permissions (camera/mic) are features, not constants.

Smaller recurring items: UI language does not follow locale and has no switch (W#98, 171, 183, 304, 310); wa.me / invite URL handling (W#3, 172, 186); font size / zoom / HiDPI (W#8, 76, 124, 192, 203, 219, 264, 276); proxy/VPN support (W#75, 223); keyboard shortcuts not configurable (W#44, 319, 28, 136); settings unreachable offline (W#223).

## Most-requested features (ranked by distinct issues)

| Rank | Feature | Count | Issues |
|---|---|---|---|
| 1 | Voice / video calls | 10 | W#43, 106, 112, 134, 187, 218, 263, 287; Y#33, 97 |
| 2 | Packaging targets: Flatpak/AppImage/AUR/Flathub/nix/Pacstall/Windows/macOS | 11 | W#74, 77, 119, 123, 125, 228, 325; Y#20, 21, 30, 85 |
| 3 | Font size / zoom factor / HiDPI scaling / font family | 8 | W#8, 76, 124, 203, 219, 264, 276, 192 |
| 4 | Multiple accounts / multiple instances | 7 | W#166, 176, 231, 259, 327, 153; Y#22 |
| 5 | Tray icon options: monochrome, hide, status-aware icon | 6 | W#93, 108, 127, 128, 193; Y#14 |
| 6 | CLI / IPC control (toggle window, unread count, raise running instance) | 6 | W#10, 14, 30, 38, 64, 165 |
| 7 | Disable "started minimized"/preview nag notifications | 4 | W#11, 180, 205, 316 (+ PRs #266, 308, 309) |
| 8 | Keyboard: configurable shortcuts, Enter=newline, up-arrow edit, shortcut docs | 4 | W#28, 44, 136, 319 |
| 9 | Native desktop notifications with click-to-chat | 4 | W#26, 96, 121, 141 |
| 10 | Multi-language spell-check / on-demand dictionaries | 4 | W#61, 132; Y#46, 76 |
| 11 | wa.me / invite URL handler | 3 | W#3, 172, 186 |
| 12 | Custom CSS / wallpaper / theming | 3 | W#182, 277, 219 |
| 13 | Blur / hide chats for privacy | 2 | W#247, 267 |
| 14 | Scheduled messages | 2 | W#94, 250 (whatly implements it; PR Y#101) |
| 15 | Collapsible sidebar | 2 | W#159, 331 (whatly PR #25 implements it) |
| 16 | Proxy support / system proxy | 2 | W#75, 223 |
| 17 | Clipboard image paste | 2 | W#33, 311 |
| 18 | Remember window position/monitor | 2 | W#103, 135 |
| 19 | Mobile / PinePhone layout | 2 | W#83, 239 |
| 20 | Others (1 each) | - | smooth scrolling W#178; hide muted status W#242; group chat folders W#140; unread badge on dock W#122; start minimized W#19; quick mute W#139; Help/About menu W#257; max refresh rate W#221; app lock W#89; TUI client W#95; Qt6 port W#168; disable GPU W#191 |

## Still open: keshavbhatt/whatsie (155)

- #338 [security/privacy] [Bug] Unlock to access Settings (2026-08-26, 0 comments)
- #336 [downloads] Can't download document (2026-08-22, 0 comments)
- #333 [tray/window] no way out when full screen pop-out window (quit app needed) (2026-08-04, 0 comments)
- #332 [theme/ui] flatpak 50/50 picking up dark theme (2026-08-04, 0 comments)
- #331 [feature-request] option to minimize side bar (2026-07-24, 1 comments)
- #330 [other] [IT IS NOT AN ISSUE] Thank you 👍 (2026-07-22, 0 comments)
- #329 [notifications] Notifications are only in the Whatsie window. (2026-07-19, 1 comments)
- #327 [feature-request] Two accounts, two windows, zero mix-ups - offering to build multi-account support (2026-07-16, 1 comments)
- #325 [packaging/distro] Proposal: free code signing for future Windows releases (SignPath Foundation) (2026-07-13, 0 comments)
- #320 [packaging/distro] Whatsie doesn't launch (Qt related issue) (2026-07-05, 25 comments)
- #319 [docs/question] What are the in app shortcuts? (2026-06-27, 1 comments)
- #318 [webengine/whatsapp-breakage] whatsie does not load whatsapp web page (2026-06-22, 5 comments)
- #316 [notifications] Feature Request: Add option to disable "started minimized in tray" popup notification (2026-05-20, 2 comments)
- #314 [packaging/distro] Unable to build (2026-05-18, 10 comments)
- #313 [webengine/whatsapp-breakage] "You received a message on your phone, but your version of WhatsApp Web doesn't support it" (2026-05-11, 2 comments)
- #312 [notifications] Chat picture in notification has colors altered (Fedora GNOME) (2026-05-04, 2 comments)
- #311 [bug] Enable pasting images from clipboard (2026-04-29, 7 comments)
- #307 [notifications] "Message notifications are off. Turn on" (2026-04-15, 1 comments)
- #306 [webengine/whatsapp-breakage] Error 429 (2026-04-09, 0 comments)
- #305 [tray/window] Window won't back to previous state from Maximized (2026-04-08, 1 comments)
- #304 [spellcheck/i18n] Translation (2026-04-06, 4 comments)
- #300 [theme/ui] Possible fix for: Checkboxes invisible in dark mode on Permissions dialog (2026-03-30, 1 comments)
- #299 [theme/ui] App does not remember selected theme after restart (2026-03-29, 7 comments)
- #292 [tray/window] WhatSie doesn't restore from tray. (2026-03-10, 1 comments)
- #291 [crash] Middle-clicking a link crashes whatsie (2026-03-08, 1 comments)
- #287 [media] Whatsapp Video and Voice Call does not worked (2026-02-05, 10 comments)
- #286 [packaging/distro] ldconfig failed, exit status 256 (2026-02-04, 0 comments)
- #285 [bug] Drag and Drop attachments not working (2026-02-03, 2 comments)
- #284 [notifications] No desktop notifictions on Ubuntu (2026-01-26, 2 comments)
- #282 [docs/question] Is the project dead? (2026-01-01, 5 comments)
- #281 [other] About text (2025-12-16, 0 comments)
- #280 [webengine/whatsapp-breakage] app does not load whatsapp messages on start-up (2025-12-03, 5 comments)
- #277 [feature-request] Feature Request (2025-11-19, 1 comments)
- #276 [theme/ui] The UI ignores system settings (2025-11-15, 1 comments)
- #275 [packaging/distro] Whatsie doesn't start after last update Arch Linux today (2025-11-11, 6 comments)
- #274 [tray/window] Main window doesn't open when clicking on Whatsie tray icon. (2025-10-12, 3 comments)
- #273 [downloads] Download directory settings not available (2025-10-12, 0 comments)
- #272 [crash] Failed to start on Fedora 42 (flatpak) (2025-10-01, 2 comments)
- #271 [notifications] Notification problem (2025-09-23, 5 comments)
- #270 [webengine/whatsapp-breakage] Doesn't open after synching messages (2025-09-22, 2 comments)
- #268 [media] KDE Neon 6 - kwin crash when trying to play video full screen (2025-09-04, 2 comments)
- #267 [feature-request] [Feature Request] Option to blur/hide messages and other content (2025-09-02, 1 comments)
- #264 [feature-request] Configurable page zoom factor (2025-08-29, 1 comments)
- #263 [feature-request] call function (2025-08-28, 1 comments)
- #262 [spellcheck/i18n] Spell Checker doesn't Work (2025-08-27, 3 comments)
- #261 [webengine/whatsapp-breakage] Server's certificate is not trusted (2025-08-14, 1 comments)
- #260 [other] warning: `whatsie' uses wireless extensions which will stop working for Wi-Fi 7 hardware; use nl80211 (2025-08-14, 1 comments)
- #259 [feature-request] Adding option (2025-08-12, 1 comments)
- #257 [feature-request] Enhancement: Add a Help menu with an "About" option that shows the version number (2025-07-07, 1 comments)
- #255 [performance] Whatsie consumes 9 GB of RAM on Ubuntu (memory leak or excessive usage) (2025-06-14, 8 comments)
- #254 [other] whatsapp (2025-06-09, 0 comments)
- #252 [bug] After start Content of main window doesnt load (2025-05-27, 4 comments)
- #251 [bug] It freezed my heart (2025-05-13, 0 comments)
- #250 [feature-request] [BE] Pseudo-programmed Messages (2025-05-08, 1 comments)
- #249 [login/session] Whatsie Stuck at 'Loading Your Chats [100%]' on Fedora (2025-05-07, 20 comments)
- #248 [crash] Does not open on Fedora 41 (2025-04-27, 2 comments)
- #246 [spellcheck/i18n] Regression: Snap revision 165 breaks spellchecking on Ubuntu 24.04 (2025-04-22, 1 comments)
- #245 [crash] Whatsie crashing when trying to link device (2025-04-18, 1 comments)
- #244 [crash] It crashes constantly (2025-04-16, 2 comments)
- #242 [feature-request] Doesn't hide muted status updates (2025-04-04, 1 comments)
- #241 [performance] Big memory consumption. 13X of Windows version. (2025-04-01, 3 comments)
- #240 [security/privacy] Says "Unlock to access Settings" not showing unlock window (2025-03-31, 1 comments)
- #239 [feature-request] Best WhatSie Integration for Linux Smartphone... (2025-03-22, 0 comments)
- #237 [theme/ui] different curson theme then system cursor theme (2025-02-22, 2 comments)
- #235 [theme/ui] Full width mode don't work on Ubuntu 24.04 LTS (2025-01-30, 5 comments)
- #234 [crash] Flatpak refuses to launch (2025-01-21, 2 comments)
- #233 [spellcheck/i18n] fcitx5 doesn't work on Ubuntu snap (2025-01-13, 2 comments)
- #232 [bug] Touchpad scrolling not work (2025-01-09, 1 comments)
- #231 [feature-request] several whatsapp accounts? (2025-01-06, 1 comments)
- #230 [bug] Accidental user home directory deletion  (2025-01-03, 5 comments)
- #229 [login/session] Stops connecting to Whatsapp after a while (2024-12-23, 8 comments)
- #228 [packaging/distro] AppImage support (2024-12-22, 2 comments)
- #227 [tray/window] Flickering window in under Wayland with flatpak on Ubuntu (2024-12-21, 1 comments)
- #226 [packaging/distro] Removing Whatsie causes /var/log/syslog to grow indefinitely (2024-12-17, 1 comments)
- #225 [crash] Repeatedly vanishing from tray on openSUSE Leap 15.5 (2024-12-10, 1 comments)
- #223 [bug] "whatsie -s" - The site can't be reached. (2024-12-02, 0 comments)
- #222 [packaging/distro] MESA-LOADER: glx: failed to open zink: driver not built! (2024-12-02, 1 comments)
- #221 [feature-request] make whatsie run at max refresh rate (2024-11-23, 2 comments)
- #220 [login/session] Issue Syncing Chats (2024-11-20, 7 comments)
- #219 [feature-request] Option to change font family (2024-11-20, 1 comments)
- #218 [feature-request] Calling System (2024-11-15, 2 comments)
- #217 [bug] Right Click Issue (2024-11-15, 2 comments)
- #216 [packaging/distro] add security flags under a separate configure option (2024-11-04, 1 comments)
- #208 [login/session] lost connection after sleep (2024-10-28, 2 comments)
- #204 [packaging/distro] Flatpack size from software manager LM (2024-10-23, 2 comments)
- #203 [theme/ui] Running Nobara 40 Wayland on 4k resolution fonts are so small  (2024-10-20, 5 comments)
- #201 [webengine/whatsapp-breakage] Whatsie don't load messages. (2024-10-12, 2 comments)
- #200 [bug] discrete graphics (2024-09-22, 1 comments)
- #196 [login/session] It's not working on Ubuntu 24.04.1 LTS (2024-09-14, 13 comments)
- #195 [webengine/whatsapp-breakage] Server certificate is not trusted. (2024-09-12, 7 comments)
- #194 [performance] Broken with memory leak (2024-09-02, 7 comments)
- #193 [tray/window] Add option to hide system tray icon (2024-08-18, 2 comments)
- #192 [tray/window] When a scaling factor ("Zoom factor when normal" in the settings) is set, the minimum window size is not multiplied by the scale factor, causing the window to be forced to be too large (2024-08-13, 3 comments)
- #191 [performance] [FR] Disable GPU acceleration and/or reduce memory usage. (2024-08-06, 1 comments)
- #190 [performance] Abnormally High CPU Thrashing (2024-08-05, 18 comments)
- #189 [tray/window] whatsie on Wayland (2024-08-04, 5 comments)
- #187 [feature-request] [Feature Request] Spoof CLient to Whatsapp for MacOS to get voice and videocall support (2024-08-01, 3 comments)
- #186 [feature-request] how to open group chat invite links on Whatsie? (2024-07-31, 0 comments)
- #185 [crash] OOM? Whatsie performs several GCs and interrupts then (2024-07-30, 1 comments)
- #184 [login/session] Whatsie receives messages but cannot send them  (2024-07-26, 13 comments)
- #183 [spellcheck/i18n] Need A Translation (2024-07-16, 1 comments)
- #182 [feature-request] Support for custom theming with provided css (2024-07-09, 1 comments)
- #181 [login/session] Can't sent message when Whatsie and Mobile app are used at the same time  (2024-06-21, 2 comments)
- #178 [feature-request] Implement smooth scrolling (2024-06-06, 3 comments)
- #176 [feature-request] when using alternative home for multi instances of whatsie cannot run 2 at the same time (2024-05-30, 2 comments)
- #175 [crash] WhatSie 4.15.0 segfaults on startup (2024-05-26, 4 comments)
- #173 [tray/window] Icon (2024-05-07, 3 comments)
- #171 [spellcheck/i18n] Dispaly Language - how to customize? (2024-04-13, 2 comments)
- #166 [feature-request] Account switching? (2024-03-16, 2 comments)
- #165 [feature-request] Get number of unread messages from CLI (2024-02-21, 1 comments)
- #162 [login/session] Whatsie sync message conversation (2024-02-09, 6 comments)
- #161 [notifications] Native notification does not work on ubuntu (2024-01-21, 4 comments)
- #160 [notifications] Native Notification doesnt work with wired on Arch Linux (2024-01-17, 4 comments)
- #157 [theme/ui] system theme gnome (2024-01-09, 2 comments)
- #155 [crash] snap doesn't start (2024-01-04, 1 comments)
- #154 [bug] Nothing but a blank screen on Wayland (2024-01-03, 14 comments)
- #153 [login/session] Multiple client stuck (2023-12-21, 3 comments)
- #145 [login/session] doesnt recieve/send messages (2023-11-18, 8 comments)
- #144 [crash] WhatSie closes itself (2023-11-18, 5 comments)
- #142 [performance] Whatsie eats up available ram during syncing. (2023-11-13, 4 comments)
- #139 [feature-request] Quickly mute/unmute? (2023-11-03, 1 comments)
- #137 [packaging/distro] Fedora Silverblue: No Files/Pictures/Attachments from Desktop accessible (2023-10-11, 1 comments)
- #136 [feature-request] Add press up arrow to edit last message (2023-09-24, 0 comments)
- #135 [tray/window] Keep window position (2023-09-12, 2 comments)
- #132 [spellcheck/i18n] Multiple languages spell checker (2023-08-26, 3 comments)
- #131 [performance] Whatsie takes too much resources (CPU/bandwidth) (2023-08-22, 11 comments)
- #128 [tray/window] Monochromatic tray icon (2023-07-07, 2 comments)
- #125 [feature-request] Feature Request: Support Windows  (2023-06-27, 8 comments)
- #124 [theme/ui] Zoom factor scale values (2023-06-14, 2 comments)
- #122 [feature-request] Add unread notification indicator to task bar (2023-06-06, 1 comments)
- #120 [notifications] no notification sound (2023-06-04, 3 comments)
- #119 [packaging/distro] Build on macos? (2023-06-01, 2 comments)
- #118 [security/privacy] Whatsie settings accessible even when app is locked (2023-05-28, 2 comments)
- #117 [theme/ui] Whatsie Loading Visual Bug (2023-05-20, 1 comments)
- #114 [webengine/whatsapp-breakage] Whatsie abruptly stops after waking up computer, needing to restart (2023-05-13, 2 comments)
- #112 [feature-request] When will we be able to make audio and video call by whatsapp for linux? (2023-04-08, 2 comments)
- #111 [bug] Can't Upload Any Attachments in Whatsie (Ubuntu 22.04) (2023-03-30, 8 comments)
- #109 [bug] WebEngineContext error while starting Whatsie (2023-03-10, 5 comments)
- #108 [tray/window] Use a different icon when Whatsie is not connected / not logged in (2023-03-07, 3 comments)
- #107 [login/session] Whatsie stops updating randomly (2023-03-06, 3 comments)
- #105 [bug] The window was totally disfigured after I switched from wayland to gdm3 on Ubuntu 22.04. (2023-02-23, 2 comments)
- #104 [performance] Whatsie and QtWebEngineProc have huge CPU requirements (2023-02-15, 4 comments)
- #103 [tray/window] [User Experience Issue] Application Always Opening At The "Main Monitor" | Only When First Time Startup (2023-02-13, 2 comments)
- #98 [spellcheck/i18n] how do i translate this to other languages? (2023-01-22, 1 comments)
- #93 [tray/window] Add monocrome tray icons (2022-11-30, 6 comments)
- #92 [login/session] cannot send messages anymore (2022-11-13, 9 comments)
- #76 [theme/ui] Feature request : add prefered font size to settings... (2022-09-13, 5 comments)
- #75 [feature-request] Looks like the webapp doesn't work when connected to vpn (2022-09-12, 1 comments)
- #66 [tray/window] Whatsie preventing system shutdown (2022-07-22, 4 comments)
- #64 [feature-request] [feature request] toggle function via command line (2022-07-15, 3 comments)
- #63 [spellcheck/i18n] Problems with character cedilha (ç) in US International keyboard layout (2022-07-09, 4 comments)
- #61 [packaging/distro] build only needed dictionaries (2022-07-08, 1 comments)
- #58 [login/session] Stuck at "organizing Messages" Screen (2022-06-28, 5 comments)
- #50 [packaging/distro] unneeded depends at build time (2022-06-23, 5 comments)
- #48 [login/session] I have to log out and re-link my device each time (2022-06-20, 7 comments)

## Still open: shakaran/whatly (10)

- #103 [docs/question] How to open the settings dialog with the flatpak version? (2026-08-27, 0 comments)
- #98 [bug] [BUG] Application hangs with ~100% CPU usage upon restart after changing settings (2026-08-20, 0 comments)
- #97 [media] Call and Video Call Buttons greyed out ("unsupported browser" message) (2026-08-20, 2 comments)
- #96 [media] suggestion: stop whatly from showing the "this image can't be uploaded in HD" for every image (make it stop the first time) (2026-08-19, 1 comments)
- #91 [bug] follow-up: off-screen unload timer restarts with a full delay for every window (#82) (2026-08-15, 0 comments)
- #85 [packaging/distro] proposal(update): the AppImage's zsync delta update is published with every release, and nothing in the app ever invokes it (2026-08-13, 1 comments)
- #84 [bug] bug: Qt Wayland rendering fails with NVIDIA on Plasma Wayland on .deb (but works when forcing X11) (2026-08-13, 4 comments)
- #71 [packaging/distro] proposal(packaging): an update-only Windows download would be 2 MB instead of 143 MB — measured across 7.0.0 → 7.1.0, where 1 of 139 files changed (2026-08-11, 1 comments)
- #43 [login/session] Unable to log in using phone number instead of QR. (2026-08-03, 12 comments)
- #20 [packaging/distro] Release on Flathub (2026-07-26, 1 comments)

## Pull requests

### whatsie

| # | State | Title | Created | Merged |
|---|---|---|---|---|
| 337 | CLOSED | feat: add resilient reply preview updater for edited messages | 2026-08-23 | - |
| 335 | OPEN | fix: restore the window from the tray when the WM minimized it | 2026-08-16 | - |
| 334 | MERGED | fix: don't force --disable-gpu, it blanks video in calls | 2026-08-08 | 2026-08-18 |
| 326 | OPEN | fix: dark theme resetting to light across restarts | 2026-07-15 | - |
| 324 | OPEN | Identify as WhatSie in the phone's linked-devices list | 2026-07-10 | - |
| 323 | MERGED | Fix logout: app hangs on "Logging out" and WhatsApp Web opens in the default browser | 2026-07-10 | 2026-07-11 |
| 322 | OPEN | Add setting: close emoji/sticker panel on outside click | 2026-07-10 | - |
| 321 | MERGED | Add Windows support (single codebase via Q_OS guards) | 2026-07-10 | 2026-08-18 |
| 309 | OPEN | Remove notification when restored existing | 2026-04-17 | - |
| 308 | OPEN | Remove notification when minimizing | 2026-04-17 | - |
| 302 | CLOSED | chore: Fetch external deps using FetchContent | 2026-03-31 | - |
| 298 | MERGED | Fix typo for Fedora | 2026-03-28 | 2026-03-29 |
| 295 | MERGED | Show notification using org.freedesktop.Notifications directly (Qt6) | 2026-03-26 | 2026-03-29 |
| 294 | MERGED |  feat:  migrate to Qt6.10 witth cmake build system | 2026-03-26 | 2026-03-26 |
| 289 | CLOSED | fix: port codebase to Qt 6 API | 2026-02-14 | - |
| 266 | OPEN | Remove notification when starting minimized | 2025-08-31 | - |
| 265 | MERGED | Fix capitalization of Qt | 2025-08-31 | 2026-01-20 |
| 258 | CLOSED | Show notification using org.freedesktop.Notifications directly | 2025-08-03 | - |
| 224 | MERGED | Add support for Debian packages | 2024-12-05 | 2026-01-20 |
| 215 | MERGED | chore: version bump to 4.16.3 | 2024-11-01 | 2024-11-01 |
| 214 | MERGED | chore: dynamically find qwebengine_convert_dict binary | 2024-11-01 | 2024-11-01 |
| 209 | MERGED | Add Qt 6 support | 2024-10-28 | 2024-11-01 |
| 202 | MERGED | fix: Notification issue on gnome shell + minor improvements | 2024-10-19 | 2024-10-19 |
| 199 | MERGED | fix: fix spell checker | 2024-09-18 | 2024-10-09 |
| 197 | CLOSED | Create devcontainer.json | 2024-09-15 | - |
| 188 | CLOSED | Add support for secure compilation flags on aarch64 (specific) and generic | 2024-08-02 | - |
| 150 | MERGED | Fix incorrect full width modification | 2023-11-30 | 2023-12-01 |
| 148 | MERGED | Remove an unnecessary line from `README.md` | 2023-11-27 | 2023-11-29 |
| 146 | MERGED | Use qmake-provided _DATE_ | 2023-11-21 | 2023-11-29 |
| 100 | MERGED | Fix icon on Plasma Wayland | 2023-01-26 | 2023-01-27 |
| 65 | MERGED | fix: hide/show window only when tray icon is left clicked | 2022-07-16 | 2022-07-22 |
| 59 | MERGED | chore(ci): update release wf | 2022-07-03 | 2022-07-03 |
| 56 | MERGED | chore: set a minimum of 4 digits for the lock code | 2022-06-25 | 2022-06-27 |
| 55 | MERGED | feat(i18n): add Italian localization | 2022-06-24 | 2022-06-25 |
| 53 | MERGED | feat: enable support for traybar entries on GNOME dash | 2022-06-24 | 2022-06-25 |
| 52 | MERGED | chore(qmake): avoid error message when .git folder is missing (close … | 2022-06-24 | 2022-06-24 |
| 45 | MERGED | fix: add missing icon, enabling install_icon target generation (clos… | 2022-06-16 | 2022-06-17 |
| 35 | MERGED | feat: v4.0 | 2022-03-29 | 2022-03-29 |
| 32 | MERGED | fix: improve download file behavior | 2022-03-22 | 2022-03-22 |
| 13 | MERGED | Decreases the main window minimum height to 500px | 2021-07-22 | 2021-07-23 |

### whatly

| # | State | Title | Created | Merged |
|---|---|---|---|---|
| 101 | OPEN | fix(scheduled): find WhatsApp's Send by its current icon name | 2026-08-24 | - |
| 100 | OPEN | fix(hd-media): find the quality control by its icon, and ask before clicking (#96) | 2026-08-24 | - |
| 95 | MERGED | fix(dictionaries): keep a Windows shortcut from passing as a dictionary | 2026-08-19 | 2026-08-23 |
| 92 | MERGED | build(msvc): define NOMINMAX before windows.h in main.cpp | 2026-08-15 | 2026-08-15 |
| 90 | MERGED | feat(media): say when a download did not arrive, and what to try | 2026-08-14 | 2026-08-15 |
| 89 | MERGED | fix(diag): only call it a loader collapse when a module id is missing (#43) | 2026-08-14 | 2026-08-15 |
| 88 | MERGED | fix(settings): consume the restart state when it is used, and stamp it with the build | 2026-08-13 | 2026-08-15 |
| 87 | MERGED | feat(settings): a search box that filters the page to what matches (#39) | 2026-08-13 | 2026-08-15 |
| 86 | MERGED | fix(settings): name the interface languages as the spell-check ones are named | 2026-08-13 | 2026-08-15 |
| 83 | MERGED | fix(theme): grey out disabled text in the dark palette too | 2026-08-13 | 2026-08-15 |
| 82 | MERGED | feat(accounts): unload the account in a window that is minimised or put away (#25) | 2026-08-13 | 2026-08-15 |
| 80 | MERGED | feat(tray): count past nine on the badge, and say the whole count in the tooltip | 2026-08-13 | 2026-08-15 |
| 79 | MERGED | feat(diag): log which WhatsApp Web build each account is running | 2026-08-13 | 2026-08-14 |
| 78 | MERGED | feat(spelling): one list for the languages — tick it, download it, delete it (#46) | 2026-08-13 | 2026-08-14 |
| 77 | MERGED | fix(settings): the wheel guard missed the double spin boxes, so a scroll could halve the whole interface | 2026-08-13 | 2026-08-13 |
| 75 | MERGED | fix(spelling): open the language list through the arrow, not through showPopup() | 2026-08-12 | 2026-08-13 |
| 73 | MERGED | feat(spelling): switch between the chosen languages while typing (#41) | 2026-08-11 | 2026-08-12 |
| 72 | MERGED | fix(settings): the wheel scrolls what the pointer is on, not always the page | 2026-08-11 | 2026-08-12 |
| 70 | MERGED | feat(update): advise per installation, not one sentence for everyone | 2026-08-11 | 2026-08-12 |
| 69 | MERGED | fix(windows): ship the MSVC runtime the executable imports (#68) | 2026-08-11 | 2026-08-12 |
| 66 | MERGED | feat(accounts): let the user say what the unread badge counts | 2026-08-09 | 2026-08-11 |
| 65 | MERGED | fix(accounts): count what is unread from WhatsApp's own store, not from its title | 2026-08-09 | 2026-08-11 |
| 64 | MERGED | build(msvc): split the two injected scripts that outgrew a string literal | 2026-08-09 | 2026-08-10 |
| 63 | MERGED | fix(accounts): switch account on the whole click, not on the press | 2026-08-09 | 2026-08-11 |
| 62 | MERGED | fix(app): keep the log running across a restart | 2026-08-08 | 2026-08-10 |
| 61 | MERGED | feat(window): say which version is running, where someone would look for it | 2026-08-08 | 2026-08-11 |
| 60 | MERGED | fix(web): put the sidebar buttons back when WhatsApp rebuilds the rail around them | 2026-08-07 | 2026-08-11 |
| 59 | MERGED | fix(media): point the codec notice at a build that exists | 2026-08-07 | 2026-08-11 |
| 58 | MERGED | fix(accounts): tear off the tab that was dragged, not the one now in its slot | 2026-08-07 | 2026-08-10 |
| 57 | MERGED | fix(accounts): an account that is not in front is still an account | 2026-08-07 | 2026-08-11 |
| 56 | MERGED | fix(settings): keep the wheel with the page, and wrap long tooltips | 2026-08-06 | 2026-08-11 |
| 55 | MERGED | feat(window): stop treating one window as the main one | 2026-08-06 | 2026-08-11 |
| 54 | MERGED | fix(chatlist): keep the preview's avatar in the top-left corner | 2026-08-06 | 2026-08-11 |
| 52 | MERGED | refactor(settings): file the "Performance & Privacy" grab-bag where each setting belongs | 2026-08-06 | 2026-08-11 |
| 51 | MERGED | fix(accounts): make the selected account tab visibly selected | 2026-08-06 | 2026-08-10 |
| 50 | MERGED | feat(chatlist): show unread counts on the collapsed strip | 2026-08-06 | 2026-08-11 |
| 49 | MERGED | feat(window): resize the custom frame from every edge and corner, and give detached windows the same frame | 2026-08-06 | 2026-08-11 |
| 48 | MERGED | perf(accounts): give an unused account no page at all, instead of a frozen one | 2026-08-05 | 2026-08-09 |
| 47 | MERGED | feat(accounts): reopen on the account that was active at exit | 2026-08-05 | 2026-08-06 |
| 45 | MERGED | packaging(flatpak): drop webenginedriver, 17 MB nobody can use | 2026-08-04 | 2026-08-06 |
| 44 | MERGED | feat(diag): log the version, commit, branch and build time at startup | 2026-08-04 | 2026-08-06 |
| 40 | CLOSED | feat(attach): read dropped files off the UI thread, behind a progress bar | 2026-08-03 | - |
| 39 | CLOSED | feat(settings): flag restart-only settings and add a per-group Restart now button | 2026-08-02 | - |
| 31 | MERGED | i18n(eo): "Advanced" is Fakula, not Progresinta | 2026-07-28 | 2026-07-28 |
| 29 | MERGED | feat(i18n): add Esperanto | 2026-07-27 | 2026-07-28 |
| 27 | MERGED | feat(settings): a Restart now button, and three things around it | 2026-07-27 | 2026-07-28 |
| 26 | MERGED | feat(window): optional account strip, and an option to hide the title bar | 2026-07-27 | 2026-07-28 |
| 25 | MERGED | feat(chats): collapse the chat list to a strip of profile pictures | 2026-07-27 | 2026-07-28 |
| 23 | MERGED | fix(tests): stop the suite writing into the real settings | 2026-07-26 | 2026-07-27 |
| 19 | MERGED | fix(windows): survive being launched from the system directory | 2026-07-26 | 2026-07-27 |
| 18 | MERGED | build: find qwebengine_convert_dict on Windows | 2026-07-26 | 2026-07-27 |
| 17 | MERGED | fix(proxy): treat a manual proxy with no host as no proxy | 2026-07-26 | 2026-07-26 |
| 16 | MERGED | fix(tests): build the logic suite on Windows (timegm) | 2026-07-26 | 2026-07-26 |
| 13 | MERGED | fix(tray): never hide the window when there is no tray icon to restore it | 2026-07-25 | 2026-07-26 |
| 10 | MERGED | Multi-window account tabs (Chrome-style) | 2026-07-24 | 2026-07-25 |
| 9 | MERGED | Redesign the settings page: collapsible sections, clearer labels, and translations | 2026-07-23 | 2026-07-24 |
| 8 | MERGED | Hide on tray-click when the window is frontmost (Windows) | 2026-07-23 | 2026-07-24 |
| 7 | MERGED | Force wa_web_show_hd_photo so HD media renders when delivered | 2026-07-23 | 2026-07-24 |
| 6 | MERGED | Don't dismiss the emoji panel when picking a skin tone | 2026-07-23 | 2026-07-24 |
| 5 | MERGED | Keep the custom notification popup on-screen (top-right) | 2026-07-23 | 2026-07-24 |
| 4 | MERGED | Use a forward slash in the default download path | 2026-07-23 | 2026-07-24 |

