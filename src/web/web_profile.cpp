#include "web/web_profile.h"

#include "core/settings/settings.h"
#include "core/spellcheck.h"
#include "web/bridge.h"
#include "web/logging.h"
#include "web/proxy.h"
#include "web/script_bundle.h"
#include "web/user_agent.h"

#include <QDir>
#include <QStandardPaths>
#include <QWebEnginePermission>
#include <QWebEngineSettings>
#include <QtEnvironmentVariables>

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
    configureSpellCheck();

    connect(&m_settings, &core::Settings::smoothScrollingChanged, this, [this](bool enabled) {
        this->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, enabled);
    });
    // Rebake the config so newly loaded pages start at the current blur level;
    // WebView applies it live to the already-loaded page.
    connect(&m_settings, &core::Settings::messageBlurLevelChanged, this, [this](int) { installBootstrap(); });
    connect(&m_settings, &core::Settings::themeChanged, this, [this](core::Theme) { installBootstrap(); });

    // FEATURES P3: proxy is process-wide; set it before the first request and
    // re-apply whenever it changes (a page reload then picks it up).
    applyProxy(m_settings.proxyConfig());
    connect(&m_settings, &core::Settings::proxyChanged, this,
            [this] { applyProxy(m_settings.proxyConfig()); });
    connect(&m_settings, &core::Settings::webrtcPublicInterfacesOnlyChanged, this, [this](bool on) {
        this->settings()->setAttribute(QWebEngineSettings::WebRTCPublicInterfacesOnly, on);
    });
    connect(&m_settings, &core::Settings::spellCheckEnabledChanged, this,
            [this](bool on) { setSpellCheckEnabled(on); });
    connect(&m_settings, &core::Settings::spellCheckLanguagesChanged, this,
            [this](const QStringList& langs) { setSpellCheckLanguages(langs); });
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

    // Pre-grant Notifications so WhatsApp never shows its "notifications are off"
    // banner (W#307); it persists reliably. Camera/microphone do NOT pre-grant
    // reliably before a request, so PermissionController grants those on first
    // request instead (respecting an explicit deny). ADR-022.
    QWebEnginePermission notifications = queryPermission(QUrl(u"https://web.whatsapp.com"_s),
                                                         QWebEnginePermission::PermissionType::Notifications);
    if (notifications.state() == QWebEnginePermission::State::Ask) {
        notifications.grant();
    }
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
    // We render our own branded error page on load failure (WebView), so turn
    // off Chromium's stock one — it looks out of place inside Whatsie.
    s->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    // FEATURES A14: user option, off by default.
    s->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, m_settings.smoothScrolling());
    // FEATURES P2: keep WebRTC off private interfaces when the user asks.
    s->setAttribute(QWebEngineSettings::WebRTCPublicInterfacesOnly, m_settings.webrtcPublicInterfacesOnly());
}

QString WebProfile::themeName(core::Theme theme)
{
    switch (theme) {
    case core::Theme::Light:
        return u"light"_s;
    case core::Theme::Dark:
        return u"dark"_s;
    case core::Theme::System:
        break;
    }
    return u"system"_s;
}

void WebProfile::configureSpellCheck()
{
    // FEATURES L1: resolve each configured language to a dictionary that is
    // actually installed (QTWEBENGINE_DICTIONARIES_PATH, set by Application) —
    // e.g. en-IN, which has no dictionary, falls back to en-GB. Feeding the
    // checker an absent language just silently disables it (owner report: the
    // UI claimed "en-IN" while spell check did nothing).
    const QStringList available =
        core::availableDictionaries(qEnvironmentVariable("QTWEBENGINE_DICTIONARIES_PATH"));
    QStringList effective;
    for (const QString& want : m_settings.spellCheckLanguages()) {
        const QString dict = core::resolveDictionary(want, available);
        if (!dict.isEmpty() && !effective.contains(dict)) {
            effective << dict;
        }
    }
    setSpellCheckEnabled(m_settings.spellCheckEnabled());
    setSpellCheckLanguages(effective);
    qCInfo(lcWeb) << "spell check" << (m_settings.spellCheckEnabled() ? "on" : "off") << "requested"
                  << m_settings.spellCheckLanguages() << "effective" << effective;
}

QString linkedDeviceLabel()
{
#if defined(Q_OS_WIN)
    return u"Whatsie for Windows"_s;
#elif defined(Q_OS_MACOS)
    return u"Whatsie for macOS"_s;
#else
    return u"Whatsie for Linux"_s;
#endif
}

void WebProfile::installBootstrap()
{
    // Config passed to injected scripts as window.__whatsie.config. theme-control.js
    // forces an explicit colorScheme at the page level; "system" lets WhatsApp
    // follow the OS prefers-color-scheme (ADR-020, revised by ADR-026).
    // linkedDeviceName brands this session in WhatsApp's linked-devices list (PR #324).
    m_scripts->installBootstrap(QJsonObject{
        {u"blurLevel"_s, m_settings.messageBlurLevel()},
        {u"colorScheme"_s, themeName(m_settings.theme())},
        {u"linkedDeviceName"_s, linkedDeviceLabel()},
    });
}

} // namespace whatsie::web
