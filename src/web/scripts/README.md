# Injected scripts

JavaScript injected into WhatsApp Web. Rules:

* One file per concern, registered in `../CMakeLists.txt` via `qt_add_resources`, installed by
  `web::ScriptBundle` on the **profile-level** script collection.
* `bootstrap.js` runs first at `DocumentCreation` with the config object and `qwebchannel.js`
  prepended by C++; it creates `window.__whatsie` (`config`, `report()`, `log()`, `bridge`).
* Every other file starts with a header block (name / purpose / depends / verified / on-fail),
  is wrapped in an IIFE with `try/catch`, and reports failures through
  `window.__whatsie.report(name, error)` → `web::Bridge::scriptFailed` → `whatsie.web.js` log.
* Only the *stable tier* is allowed: standard APIs, our own CSS, structural anchors
  (`#pane-side`, `#main`, `#side`, `footer [contenteditable]`). No `window.require('WAWeb…')`,
  no React internals, no IndexedDB schema, no localised text.
* Each script maps to one approved feature; nothing speculative.

| File | Injection | Feature |
|---|---|---|
| `bootstrap.js` | DocumentCreation (bundled with config + qwebchannel.js) | infrastructure |
| `theme-control.js` | DocumentCreation (bundled) | A1 (page-level theme, ADR-026) |
| `nav-settings.js` | DocumentCreation (bundled) | A11 (Settings button in the nav rail, ADR-028) |
| `storage-persist.js` | DocumentCreation (bundled) | S1 |
