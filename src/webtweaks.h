#pragma once

#include <QString>
#include <QWebEngineProfile>

// Optional, user-toggleable JavaScript tweaks injected into WhatsApp Web to
// adjust behaviors of the web app itself (settings group "webtweaks").
// Every tweak is defensive: when WhatsApp Web changes its internals the tweak
// silently stops matching instead of breaking the page.
namespace WebTweaks {

// The userscript, with the current QSettings baked into its FLAGS object.
// Used both for profile injection (fresh page loads) and for applying a
// toggle to an already-loaded page via QWebEnginePage::runJavaScript.
QString scriptSource();

// (Re)installs the userscript on the profile for FUTURE page loads according
// to the current QSettings; removes it when nothing is enabled. Note: Qt does
// not propagate profile-script changes to an already-created page, so callers
// must also runJavaScript(scriptSource()) on the live page to apply a toggle
// immediately.
void install(QWebEngineProfile *profile);

} // namespace WebTweaks
