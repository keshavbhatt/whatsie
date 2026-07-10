#include "linkeddevicename.h"
#include "settingsmanager.h"

#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

static const char kScriptName[] = "whatsie-linked-device-name";

// __NAME__ becomes a JS string literal ("WhatSie for Linux", or "" when the
// setting is off). The label comes from WAWebBrowserInfo — a module whose
// default export is a 0-arg function returning {os, name, version, ua} — so
// the hook swaps the module registry's defaultExport for a decorating wrapper
// (older WhatsApp Web builds exposed the same function as
// WAWebMiscBrowserUtils.info; that path is kept as a fallback).
//
// The phone renders the label as "Browser (OS)", where the os field displays
// arbitrary text but the browser name is validated against known browsers
// and silently omitted otherwise (both behaviors observed by re-linking a
// phone). The wrapper exploits that: os carries the full brand text and name
// is set to the unrecognized "WhatSie", so the browser prefix disappears and
// the label reads just "WhatSie for Linux".
//
// The wrapper reads the LIVE name from window.__whatsieLinkedDeviceName, so
// re-running this script on a loaded page toggles the override without a
// reload. The module system loads asynchronously, so the hook retries until
// it resolves.
static const char kScriptTemplate[] = R"JS(
(function () {
  'use strict';
  var NAME = __NAME__;
  window.__whatsieLinkedDeviceName = NAME;  // live value read by the wrapper
  if (window.__whatsieLinkedDeviceNameHooked || !NAME) return;
  var wrap = function (orig) {
    return function () {
      var inf = orig.apply(this, arguments);
      try {
        if (window.__whatsieLinkedDeviceName && inf) {
          inf.os = window.__whatsieLinkedDeviceName;
          inf.name = 'WhatSie';  // unknown browser → phone omits the prefix
        }
      } catch (e) { /* keep the stock label */ }
      return inf;
    };
  };
  var tries = 0;
  var hook = function () {
    if (window.__whatsieLinkedDeviceNameHooked) return;
    try {
      if (typeof window.require === 'function') {
        // Current builds: bare-function module, patched via the registry.
        var rec = window.require('__debug').modulesMap['WAWebBrowserInfo'];
        if (rec && typeof rec.defaultExport === 'function') {
          var wrapped = wrap(rec.defaultExport);
          rec.defaultExport = wrapped;
          if (rec.exports && typeof rec.exports.default === 'function')
            rec.exports.default = wrapped;
          window.__whatsieLinkedDeviceNameHooked = true;
          return;
        }
        // Older builds: plain export on WAWebMiscBrowserUtils.
        var mod = window.require('WAWebMiscBrowserUtils');
        if (mod && typeof mod.info === 'function') {
          mod.info = wrap(mod.info);
          window.__whatsieLinkedDeviceNameHooked = true;
          return;
        }
      }
    } catch (e) { /* module not registered yet — retry */ }
    if (++tries < 120) setTimeout(hook, 250);  // ~30s while the app boots
  };
  hook();
})();
)JS";

namespace LinkedDeviceName {

static QString platformDeviceName() {
#if defined(Q_OS_WIN)
  return QStringLiteral("WhatSie for Windows");
#elif defined(Q_OS_MACOS)
  return QStringLiteral("WhatSie for macOS");
#else
  return QStringLiteral("WhatSie for Linux");
#endif
}

static bool isEnabled() {
  return SettingsManager::instance()
      .settings()
      .value(QStringLiteral("identifyInLinkedDevices"), true)
      .toBool();
}

QString scriptSource() {
  const QString name = isEnabled() ? platformDeviceName() : QString();

  QString source = QString::fromLatin1(kScriptTemplate);
  source.replace(QLatin1String("__NAME__"),
                 QStringLiteral("\"%1\"").arg(name));
  return source;
}

void install(QWebEngineProfile *profile) {
  auto *scripts = profile->scripts();
  const auto existing = scripts->find(QLatin1String(kScriptName));
  for (const auto &script : existing)
    scripts->remove(script);

  if (!isEnabled())
    return; // disabled → do not inject on fresh loads

  QWebEngineScript script;
  script.setName(QLatin1String(kScriptName));
  script.setSourceCode(scriptSource());
  script.setInjectionPoint(QWebEngineScript::DocumentReady);
  script.setWorldId(QWebEngineScript::MainWorld);
  script.setRunsOnSubFrames(false);
  scripts->insert(script);
}

} // namespace LinkedDeviceName
