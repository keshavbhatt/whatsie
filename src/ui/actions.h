#pragma once

#include <QList>
#include <QObject>

class QAction;
class QWidget;

namespace whatsie::ui {

/// All user-triggerable actions with their shortcuts, created once and shared
/// by the window, the tray menu and (M5) the shortcuts sheet. The owner
/// connects behaviour; this class only declares.
class Actions : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Actions)

public:
    explicit Actions(QWidget* owner);
    ~Actions() override = default;

    QAction* showHide = nullptr;
    QAction* newChat = nullptr;
    QAction* reload = nullptr;
    QAction* downloads = nullptr;
    QAction* mute = nullptr;
    QAction* blurMessages = nullptr;
    QAction* zoomIn = nullptr;
    QAction* zoomOut = nullptr;
    QAction* zoomReset = nullptr;
    QAction* fullScreen = nullptr;
    QAction* settings = nullptr;
    QAction* shortcuts = nullptr;
    QAction* about = nullptr;
    QAction* quit = nullptr;

    /// Every action, in menu order (for the tray and the shortcuts sheet).
    [[nodiscard]] QList<QAction*> all() const;
};

} // namespace whatsie::ui
