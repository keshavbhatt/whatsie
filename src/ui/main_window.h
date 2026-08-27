#pragma once

#include <QMainWindow>
#include <QPointer>

class QWebEngineDesktopMediaRequest;
class QAuthenticator;
class QStackedWidget;
class QTimer;

namespace whatsie::core {
class DndController;
class Settings;
class ThemeService;
} // namespace whatsie::core
namespace whatsie::web {
class WebView;
}

namespace whatsie::ui {

class Actions;
class DownloadsHub;
class NotificationHub;
class LockScreen;
class SettingsDialog;
class ThemeApplier;
class TrayController;

/// Top-level window: lays out the web view, owns the tray and actions, and
/// implements the window-level behaviours of FEATURES S2–S7. Everything else
/// is delegated to hubs/controllers.
class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MainWindow)

public:
    MainWindow(core::Settings& settings, core::ThemeService& theme, QWidget* parent = nullptr);
    ~MainWindow() override;

    /// Show the window unless `startHidden` and a tray exists (FEATURES S4, S5).
    void start(bool startHidden);

public Q_SLOTS:
    void showAndRaise();
    void toggleVisibility();
    /// Phone number or chat link; invalid input shows a message.
    void openChat(const QString& target);
    void promptNewChat();
    void showSettings();
    void showShortcuts();
    void toggleTheme();
    void showAbout();
    void quit();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void setupUi();
    void connectActions();
    void connectWebView();
    void restoreWindowState();
    void saveWindowState();
    void updateZoomMode();
    void applyMinimumSize();
    void setFullScreenMode(bool on);
    void handleUnread(int count);
    void handleRenderProcessGaveUp();
    void handleDesktopMediaRequest(QWebEngineDesktopMediaRequest request);
    void handleProxyAuth(const QString& proxyHost, QAuthenticator* authenticator);
    void syncAutostart();
    void setupLock();
    void requestLock();
    void lock();
    void unlock();
    void attemptUnlock(const QString& passcode);
    void tickThrottle();
    void updateIdleTimer();
    [[nodiscard]] bool isLocked() const { return m_locked; }
    void clearCache();
    void confirmClearSession();

    core::Settings& m_settings;
    core::ThemeService& m_theme;
    web::WebView* m_webView = nullptr;
    Actions* m_actions = nullptr;
    core::DndController* m_dnd = nullptr;
    TrayController* m_tray = nullptr;
    NotificationHub* m_notifications = nullptr;
    DownloadsHub* m_downloads = nullptr;
    ThemeApplier* m_themeApplier = nullptr;
    QPointer<SettingsDialog> m_settingsDialog;
    Qt::WindowStates m_stateBeforeFullScreen = Qt::WindowNoState;
    bool m_quitting = false;

    // App lock (FEATURES P1)
    QStackedWidget* m_stack = nullptr;
    LockScreen* m_lockScreen = nullptr;
    QTimer* m_idleTimer = nullptr;
    QTimer* m_throttleTimer = nullptr;
    int m_failedAttempts = 0;
    int m_throttleRemaining = 0;
    bool m_locked = false;
};

} // namespace whatsie::ui
