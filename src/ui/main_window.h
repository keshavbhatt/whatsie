#pragma once

#include <QMainWindow>

namespace whatsie::core {
class Settings;
}
namespace whatsie::web {
class WebView;
}

namespace whatsie::ui {

/// Top-level window. Deliberately thin: it lays out widgets, persists its own
/// geometry and delegates everything else to controllers that are added as
/// features are approved (tray, notifications, lock, ...).
class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MainWindow)

public:
    explicit MainWindow(core::Settings& settings, QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUi();
    void restoreWindowState();
    void saveWindowState();

    core::Settings& m_settings;
    web::WebView* m_webView = nullptr;
};

} // namespace whatsie::ui
