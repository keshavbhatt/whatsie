#pragma once

#include "core/connection_watchdog_policy.h"
#include "core/navigation_policy.h"
#include "core/render_crash_policy.h"

#include <QElapsedTimer>
#include <QWebEngineDesktopMediaRequest>
#include <QWebEnginePermission>
#include <QWebEngineView>

namespace whatsie::core {
class Settings;
class ThemeService;
} // namespace whatsie::core

namespace whatsie::web {

class PermissionController;
class WebPage;
class WebProfile;

/// The widget that shows WhatsApp Web. Owns the profile and page; exposes
/// only what the UI layer needs (zoom mode, unread count, chat links, crash
/// recovery, fullscreen, permission prompts, screen-share picks).
class WebView : public QWebEngineView
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WebView)

public:
    WebView(core::Settings& settings, core::ThemeService& theme, QWidget* parent = nullptr);
    ~WebView() override;

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
    [[nodiscard]] WebProfile& profile() { return *m_profile; }
    [[nodiscard]] WebPage& webPage() { return *m_page; }
    [[nodiscard]] PermissionController& permissions() { return *m_permissions; }
    /// Close any popped-out windows (calls) — used when locking (FEATURES P1).
    void closePopups();

Q_SIGNALS:
    void unreadCountChanged(int count);
    void fullScreenRequested(bool on);
    /// The render process died repeatedly; the UI should ask the user.
    void renderProcessGaveUp();
    void permissionPromptRequested(QWebEnginePermission permission);
    void desktopMediaRequested(QWebEngineDesktopMediaRequest request);
    /// WhatsApp Web socket up (true) or down (false), for tray dimming (T6).
    void connectionChanged(bool up);
    /// The injected in-page settings button was clicked (FEATURES A11).
    void settingsRequested();
    /// The proxy challenged and no credentials are cached (FEATURES M12b).
    void proxyAuthenticationRequired(const QString& proxyHost, QAuthenticator* authenticator);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void childEvent(QChildEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void applyZoom();
    void refreshColorScheme();
    void wirePopup(QWidget* window);
    void handleTitleChanged(const QString& title);
    void handleRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, int exitCode);
    void handleConnectionChanged(bool up);
    void checkWatchdog();
    void applyBlurLive();
    void applyThemeLive();
    void handleProxyAuth(QAuthenticator* authenticator, const QString& proxyHost);
    bool maybeHandleDrop(class QObject* watched, class QEvent* event);
    void handleFileDrop(const QStringList& paths);

    core::Settings& m_settings;
    WebProfile* m_profile = nullptr;
    WebPage* m_page = nullptr;
    PermissionController* m_permissions = nullptr;
    core::RenderCrashPolicy m_crashPolicy;
    core::ConnectionWatchdogPolicy m_watchdog;
    class QTimer* m_watchdogTimer = nullptr;
    QElapsedTimer m_clock;
    bool m_maximizedMode = false;
    int m_unread = 0;
};

} // namespace whatsie::web
