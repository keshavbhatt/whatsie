#pragma once

#include "core/navigation_policy.h"
#include "core/render_crash_policy.h"

#include <QElapsedTimer>
#include <QWebEngineView>

namespace whatsie::core {
class Settings;
}

namespace whatsie::web {

class WebPage;
class WebProfile;

/// The widget that shows WhatsApp Web. Owns the profile and page; exposes
/// only what the UI layer needs (zoom mode, unread count, chat links, crash
/// recovery, fullscreen requests).
class WebView : public QWebEngineView
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WebView)

public:
    explicit WebView(core::Settings& settings, QWidget* parent = nullptr);
    ~WebView() override = default;

    /// Navigates to WhatsApp Web (idempotent).
    void loadWhatsApp();
    /// Opens a chat via https://web.whatsapp.com/send?phone=… (FEATURES S10).
    void openChat(const core::NewChatRequest& request);

    /// Selects which zoom setting applies (normal vs maximized/fullscreen).
    void setZoomMode(bool maximized);
    void zoomStep(int direction); ///< +1 / -1, persists to the active setting
    void zoomReset();

    [[nodiscard]] int unreadCount() const { return m_unread; }
    [[nodiscard]] QString userAgent() const;

Q_SIGNALS:
    void unreadCountChanged(int count);
    void fullScreenRequested(bool on);
    /// The render process died repeatedly; the UI should ask the user.
    void renderProcessGaveUp();
    void inAppPopupRequested(const QUrl& url);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void childEvent(QChildEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void applyZoom();
    void handleTitleChanged(const QString& title);
    void handleRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, int exitCode);

    core::Settings& m_settings;
    WebProfile* m_profile = nullptr;
    WebPage* m_page = nullptr;
    core::RenderCrashPolicy m_crashPolicy;
    QElapsedTimer m_clock;
    bool m_maximizedMode = false;
    int m_unread = 0;
};

} // namespace whatsie::web
