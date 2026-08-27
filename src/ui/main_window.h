#pragma once

#include <QMainWindow>
#include <QPointer>

namespace whatsie::core {
class Settings;
class ThemeService;
} // namespace whatsie::core
namespace whatsie::web {
class WebView;
}

namespace whatsie::ui {

class Actions;
class SettingsDialog;
class ThemeApplier;
class TrayController;

/// Top-level window: lays out the web view, owns the tray and actions, and
/// implements the window-level behaviours of FEATURES S2–S7. Everything else
/// is delegated.
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
    void showAbout();
    void quit();

protected:
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

    core::Settings& m_settings;
    core::ThemeService& m_theme;
    web::WebView* m_webView = nullptr;
    Actions* m_actions = nullptr;
    TrayController* m_tray = nullptr;
    ThemeApplier* m_themeApplier = nullptr;
    QPointer<SettingsDialog> m_settingsDialog;
    Qt::WindowStates m_stateBeforeFullScreen = Qt::WindowNoState;
    bool m_quitting = false;
};

} // namespace whatsie::ui
