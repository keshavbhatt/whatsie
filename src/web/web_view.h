#pragma once

#include <QWebEngineView>

namespace whatsie::core {
class Settings;
}

namespace whatsie::web {

class WebPage;
class WebProfile;

/// The widget that shows WhatsApp Web. Owns the profile and page; exposes
/// only what the UI layer needs.
class WebView : public QWebEngineView
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WebView)

public:
    explicit WebView(core::Settings& settings, QWidget* parent = nullptr);
    ~WebView() override = default;

    /// Navigates to WhatsApp Web (idempotent).
    void loadWhatsApp();

private:
    core::Settings& m_settings;
    WebProfile* m_profile = nullptr;
    WebPage* m_page = nullptr;
};

} // namespace whatsie::web
