#include "webengineprofilemanager.h"
#include "common.h"
#include "settingsmanager.h"

#include <QDebug>
#include <QStandardPaths>
#include <QWebEngineScriptCollection>

WebEngineProfileManager &WebEngineProfileManager::instance() {
    static WebEngineProfileManager inst;
    return inst;
}

WebEngineProfileManager::WebEngineProfileManager() {
    // Named profile → persistent storage is enabled automatically.
    m_profile = new QWebEngineProfile(QStringLiteral("whatsie"));

    const QString dataPath  = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

    m_profile->setPersistentStoragePath(dataPath  + QStringLiteral("/QtWebEngine"));
    m_profile->setCachePath(cachePath + QStringLiteral("/QtWebEngine"));
    m_profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

    qDebug() << "WebEngineProfile persistent storage:" << m_profile->persistentStoragePath();
    qDebug() << "WebEngineProfile cache path:"         << m_profile->cachePath();

    auto *s = m_profile->settings();
    s->setAttribute(QWebEngineSettings::AutoLoadImages,                    true);
    s->setAttribute(QWebEngineSettings::JavascriptEnabled,                 true);
    s->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows,          true);
    s->setAttribute(QWebEngineSettings::LocalStorageEnabled,               true);
    s->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls,   true);
    s->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls,     true);
    s->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled,             false);
    s->setAttribute(QWebEngineSettings::DnsPrefetchEnabled,                true);
    s->setAttribute(QWebEngineSettings::FullScreenSupportEnabled,          true);
    s->setAttribute(QWebEngineSettings::LinksIncludedInFocusChain,         false);
    s->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled,          false);
    s->setAttribute(QWebEngineSettings::SpatialNavigationEnabled,          true);
    s->setAttribute(QWebEngineSettings::JavascriptCanPaste,                true);
    s->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard,      true);

    applyUserSettings();

    // WhatsApp Web calls navigator.storage.persist() at startup and logs a
    // non-fatal error when it returns false (QtWebEngine never auto-grants it).
    // Intercept the call at document-creation time — before any page JS runs —
    // so the promise always resolves true.  This also suppresses the follow-on
    // "[storage] storage bucket persistence denied" console spam.
    QWebEngineScript persistScript;
    persistScript.setName(QStringLiteral("grant-storage-persistence"));
    persistScript.setSourceCode(QStringLiteral(
        "(function(){"
        "  if(navigator.storage&&navigator.storage.persist){"
        "    Object.defineProperty(navigator.storage,'persist',{"
        "      value:function(){return Promise.resolve(true);},"
        "      writable:true,configurable:true"
        "    });"
        "  }"
        "  if(navigator.storage&&navigator.storage.persisted){"
        "    Object.defineProperty(navigator.storage,'persisted',{"
        "      value:function(){return Promise.resolve(true);},"
        "      writable:true,configurable:true"
        "    });"
        "  }"
        "})();"));
    persistScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
    persistScript.setWorldId(QWebEngineScript::MainWorld);
    persistScript.setRunsOnSubFrames(false);
    m_profile->scripts()->insert(persistScript);
}

WebEngineProfileManager::~WebEngineProfileManager() {
    delete m_profile;
}

QWebEngineProfile *WebEngineProfileManager::profile() const {
    return m_profile;
}

void WebEngineProfileManager::applyUserSettings() {
    QSettings &s = SettingsManager::instance().settings();

    m_profile->setHttpUserAgent(
        s.value(QStringLiteral("useragent"), defaultUserAgentStr).toString());

    m_profile->settings()->setAttribute(
        QWebEngineSettings::PlaybackRequiresUserGesture,
        s.value(QStringLiteral("autoPlayMedia"), false).toBool());
}
