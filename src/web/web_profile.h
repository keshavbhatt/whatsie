#pragma once

#include <QString>
#include <QWebEngineProfile>

#include <memory>

namespace whatsie::core {
class Settings;
enum class Theme;
} // namespace whatsie::core

namespace whatsie::web {

class Bridge;
class ScriptBundle;

/// The brand label shown in WhatsApp's linked-devices list, e.g.
/// "Whatsie for Linux" (PR #324). Platform-specific.
[[nodiscard]] QString linkedDeviceLabel();

/// The one persistent profile of the application. Storage paths, cookie
/// policy, user agent, engine attributes and the injected script bundle are
/// configured here and nowhere else (ADR-006: profile-level scripts only).
class WebProfile : public QWebEngineProfile
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WebProfile)

public:
    explicit WebProfile(core::Settings& settings, QObject* parent = nullptr);
    ~WebProfile() override;

    [[nodiscard]] core::Settings& appSettings() { return m_settings; }
    [[nodiscard]] Bridge& bridge() { return *m_bridge; }
    [[nodiscard]] ScriptBundle& scripts() { return *m_scripts; }

    /// Directory roots that storage clean-up is allowed to touch.
    [[nodiscard]] QStringList storageRoots() const;

private:
    void configureStorage();
    void configureUserAgent();
    void configureAttributes();
    void installBootstrap();
    void configureSpellCheck();
    static QString themeName(core::Theme theme);

    core::Settings& m_settings;
    std::unique_ptr<Bridge> m_bridge;
    std::unique_ptr<ScriptBundle> m_scripts;
};

} // namespace whatsie::web
