# Injected scripts

JavaScript injected into WhatsApp Web. Rules (see `DOCS/CODING_STANDARDS.md` §7):

* One file per concern, registered in `../CMakeLists.txt` via `qt_add_resources`.
* Injected through the **profile-level** `QWebEngineScriptCollection` with an
  explicit injection point, world id and `runsOnSubFrames`.
* Every file starts with a header block:

  ```js
  // name:     unread-badge
  // purpose:  report unread count changes to C++ (bridge.unreadCountChanged)
  // depends:  document.title format "(N) WhatsApp"
  // verified: 2026-08-27 against WhatsApp Web 2.3000.x
  // on-fail:  no-op; reports via bridge.scriptFailed
  ```
* Wrapped in an IIFE with `try/catch`; failures go to `window.__whatsie.bridge.scriptFailed`.
* No script is added before its feature row in `DOCS/FEATURES.md` is approved.
