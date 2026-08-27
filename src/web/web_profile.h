#pragma once

#include <QWebEngineProfile>

#include <memory>

namespace whatsie::core {
class Settings;
class ThemeService;
} // namespace whatsie::core

namespace whatsie::web {

class Bridge;
class ScriptBundle;

/// The one persistent profile of the application. Storage paths, cookie
/// policy, user agent, engine attributes and the injected script bundle are
/// configured here and nowhere else (ADR-006: profile-level scripts only).
class WebProfile : public QWebEngineProfile
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WebProfile)

public:
    WebProfile(core::Settings& settings, core::ThemeService& theme, QObject* parent = nullptr);
    ~WebProfile() override;

    [[nodiscard]] core::Settings& appSettings() { return m_settings; }
    [[nodiscard]] Bridge& bridge() { return *m_bridge; }
    [[nodiscard]] ScriptBundle& scripts() { return *m_scripts; }

    /// Directory roots that storage clean-up is allowed to touch.
    [[nodiscard]] QStringList storageRoots() const;

Q_SIGNALS:
    /// The bootstrap bundle changed (theme flipped); pages should reload.
    void bootstrapChanged();

private:
    void configureStorage();
    void configureUserAgent();
    void configureAttributes();
    void installBootstrap();

    core::Settings& m_settings;
    core::ThemeService& m_theme;
    std::unique_ptr<Bridge> m_bridge;
    std::unique_ptr<ScriptBundle> m_scripts;
};

} // namespace whatsie::web
