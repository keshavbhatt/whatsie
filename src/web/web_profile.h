#pragma once

#include <QWebEngineProfile>

namespace whatsie::core {
class Settings;
}

namespace whatsie::web {

/// The one persistent profile of the application. Storage paths, cookie
/// policy, user agent and engine attributes are configured here and nowhere
/// else. Profile-level script collection is used for injected scripts
/// (page-level DocumentCreation scripts race navigations on Qt 6.11 —
/// see DOCS/CODING_STANDARDS.md §7).
class WebProfile : public QWebEngineProfile
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WebProfile)

public:
    explicit WebProfile(core::Settings& settings, QObject* parent = nullptr);
    ~WebProfile() override = default;

private:
    void configureStorage();
    void configureUserAgent();
    void configureAttributes();

    core::Settings& m_settings;
};

} // namespace whatsie::web
