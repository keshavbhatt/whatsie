#include "web/web_profile.h"

#include "core/settings/settings.h"
#include "web/bridge.h"
#include "web/logging.h"
#include "web/script_bundle.h"
#include "web/user_agent.h"

#include <QDir>
#include <QStandardPaths>
#include <QWebEnginePermission>
#include <QWebEngineSettings>

using namespace Qt::StringLiterals;

namespace whatsie::web {

namespace {
constexpr auto kProfileName = "whatsie";
} // namespace

WebProfile::WebProfile(core::Settings& settings, QObject* parent)
    : QWebEngineProfile(QString::fromLatin1(kProfileName), parent)
    , m_settings(settings)
    , m_bridge(std::make_unique<Bridge>())
    , m_scripts(std::make_unique<ScriptBundle>(*this))
{
    configureStorage();
    configureUserAgent();
    configureAttributes();
    installBootstrap();

    connect(&m_settings, &core::Settings::smoothScrollingChanged, this, [this](bool enabled) {
        this->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, enabled);
    });
    qCInfo(lcWeb) << "profile ready, storage at" << persistentStoragePath();
}

WebProfile::~WebProfile() = default;

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

    // FEATURES N11: pre-grant so WhatsApp never shows its "notifications are
    // off" banner (W#307).
    queryPermission(QUrl(u"https://web.whatsapp.com"_s), QWebEnginePermission::PermissionType::Notifications)
        .grant();
}

QStringList WebProfile::storageRoots() const
{
    return {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation),
            QStandardPaths::writableLocation(QStandardPaths::CacheLocation)};
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
    s->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    s->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    // FEATURES A14: user option, off by default.
    s->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, m_settings.smoothScrolling());
}

void WebProfile::installBootstrap()
{
    // Config keys are added as scripts need them; the theme is NOT one of
    // them (WhatsApp follows prefers-color-scheme, see ThemeApplier / ADR-020).
    m_scripts->installBootstrap(QJsonObject{});
}

} // namespace whatsie::web
