#pragma once

#include <QObject>

#include <memory>

namespace whatsie::core {
class DndController;
class INotifier;
class NotificationService;
class Settings;
} // namespace whatsie::core
namespace whatsie::web {
class NotificationPresenter;
class WebView;
} // namespace whatsie::web

namespace whatsie::ui {

class TrayController;

/// Wires the notification stack: platform backend (or tray fallback),
/// service, and the WebEngine presenter. Keeps MainWindow thin.
class NotificationHub : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(NotificationHub)

public:
    NotificationHub(core::Settings& settings, core::DndController& dnd, TrayController& tray,
                    web::WebView& view, QObject* parent = nullptr);
    ~NotificationHub() override;

    [[nodiscard]] core::NotificationService& service() { return *m_service; }
    /// Settings → "Send test notification".
    void sendTest();

Q_SIGNALS:
    /// A notification was clicked; the window should come to the front.
    void activated();

private:
    std::unique_ptr<core::INotifier> m_platformNotifier;
    std::unique_ptr<core::INotifier> m_trayNotifier;
    std::unique_ptr<core::NotificationService> m_service;
    std::unique_ptr<web::NotificationPresenter> m_presenter;
    quint64 m_testId = 0;
};

} // namespace whatsie::ui
