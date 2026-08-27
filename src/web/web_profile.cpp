#include "web/web_profile.h"

#include "core/settings/settings.h"
#include "web/logging.h"
#include "web/user_agent.h"

#include <QDir>
#include <QStandardPaths>
#include <QWebEngineSettings>

using namespace Qt::StringLiterals;

namespace whatsie::web {

namespace {
constexpr auto kProfileName = "whatsie";
} // namespace

WebProfile::WebProfile(core::Settings& settings, QObject* parent)
    : QWebEngineProfile(QString::fromLatin1(kProfileName), parent)
    , m_settings(settings)
{
    configureStorage();
    configureUserAgent();
    configureAttributes();
    qCInfo(lcWeb) << "profile ready, storage at" << persistentStoragePath();
}

void WebProfile::configureStorage()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString storage = base + u"/profile"_s;
    const QString cache = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u"/profile"_s;
    QDir().mkpath(storage);
    QDir().mkpath(cache);

    setPersistentStoragePath(storage);
    setCachePath(cache);
    setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    setPersistentPermissionsPolicy(QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
}

void WebProfile::configureUserAgent()
{
    // The override setting lands with the "custom user agent" feature; until
    // then only the sanitized engine default is used.
    setHttpUserAgent(effectiveUserAgent(httpUserAgent(), QString()));
    qCDebug(lcWeb) << "user agent:" << httpUserAgent();
}

void WebProfile::configureAttributes()
{
    QWebEngineSettings* s = settings();
    s->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    s->setAttribute(QWebEngineSettings::JavascriptCanPaste, true);
    s->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    s->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    s->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
    s->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    s->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);
    s->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
}

} // namespace whatsie::web
