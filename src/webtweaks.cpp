#include "webtweaks.h"
#include "settingsmanager.h"

#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

static const char kScriptName[] = "whatsie-web-tweaks";

// __FLAGS__ becomes a JSON object of the enabled tweaks. Behavior is gated by
// the LIVE flags object (window.__whatsieWebTweaks) rather than a captured
// boolean, so re-running this script on a loaded page toggles the tweak
// without a reload. Every DOM access is wrapped so a stale selector can never
// break the page.
static const char kScriptTemplate[] = R"JS(
(function () {
  'use strict';
  var FLAGS = __FLAGS__;
  var W = window.__whatsieWebTweaks;
  if (W) {
    W.dismissExpressionsPanel = FLAGS.dismissExpressionsPanel;  // live update
  } else {
    W = window.__whatsieWebTweaks = FLAGS;
  }
  if (window.__whatsieWebTweaksReady) return;  // install listeners only once
  window.__whatsieWebTweaksReady = true;

  // ── Dismiss the expressions (emoji/GIF/sticker) panel on outside click. ──
  // WhatsApp keeps it open until its button is pressed again. The panel mount
  // node '#expressions-panel-container' always exists (empty, height 0); it is
  // OPEN exactly when its child has a subtree. Close via a synthetic Escape.
  var isOpen = function (c) {
    return !!(c && c.firstElementChild && c.firstElementChild.childElementCount > 0);
  };
  document.addEventListener('pointerdown', function (ev) {
    try {
      if (!W.dismissExpressionsPanel) return;
      var panel = document.querySelector('#expressions-panel-container');
      if (!isOpen(panel) || panel.contains(ev.target)) return;
      var btn = ev.target.closest && ev.target.closest('button');
      if (btn && /emoji|gif|sticker/i.test(btn.getAttribute('aria-label') || ''))
        return;
      document.dispatchEvent(new KeyboardEvent('keydown', {
        key: 'Escape', code: 'Escape', keyCode: 27, which: 27, bubbles: true,
      }));
    } catch (e) { /* never break the page */ }
  }, true);
})();
)JS";

namespace WebTweaks {

QString scriptSource() {
  QSettings &s = SettingsManager::instance().settings();
  const bool dismiss =
      s.value(QStringLiteral("webtweaks/dismissExpressionsPanel"), false).toBool();

  const QString flags = QStringLiteral("{\"dismissExpressionsPanel\":%1}")
      .arg(dismiss ? QStringLiteral("true") : QStringLiteral("false"));

  QString source = QString::fromLatin1(kScriptTemplate);
  source.replace(QLatin1String("__FLAGS__"), flags);
  return source;
}

void install(QWebEngineProfile *profile) {
  auto *scripts = profile->scripts();
  const auto existing = scripts->find(QLatin1String(kScriptName));
  for (const auto &script : existing)
    scripts->remove(script);

  QSettings &s = SettingsManager::instance().settings();
  const bool dismiss =
      s.value(QStringLiteral("webtweaks/dismissExpressionsPanel"), false).toBool();
  if (!dismiss)
    return; // nothing enabled → do not inject on fresh loads

  QWebEngineScript script;
  script.setName(QLatin1String(kScriptName));
  script.setSourceCode(scriptSource());
  script.setInjectionPoint(QWebEngineScript::DocumentReady);
  script.setWorldId(QWebEngineScript::MainWorld);
  script.setRunsOnSubFrames(false);
  scripts->insert(script);
}

} // namespace WebTweaks
