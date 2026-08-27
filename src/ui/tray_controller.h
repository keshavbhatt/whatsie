#pragma once

#include <QIcon>
#include <QImage>
#include <QObject>

class QAction;
class QActionGroup;
class QMenu;
class QSystemTrayIcon;

namespace whatsie::core {
class DndController;
class Settings;
class ThemeService;
} // namespace whatsie::core

namespace whatsie::ui {

class Actions;

/// System tray icon with unread badge, DND submenu and context menu
/// (FEATURES T1, T2, T4, N7). Never the only way back to the window: callers
/// must check isAvailable() before hiding (FEATURES S5).
class TrayController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(TrayController)

public:
    TrayController(core::Settings& settings, core::ThemeService& theme, core::DndController& dnd,
                   Actions& actions, QObject* parent = nullptr);
    ~TrayController() override;

    [[nodiscard]] bool isAvailable() const;
    [[nodiscard]] QSystemTrayIcon* systemTrayIcon() const { return m_tray; }
    void setUnreadCount(int count);
    /// Called by the window when its visibility changes, to relabel the
    /// show/hide entry.
    void setWindowVisible(bool visible);
    /// Connection state from the web layer; dims the icon while offline when
    /// the "dim when disconnected" setting is on (FEATURES T6).
    void setConnected(bool connected);

    /// Icon with the current badge, for the window as well.
    [[nodiscard]] QIcon currentIcon() const { return m_icon; }

Q_SIGNALS:
    /// Left click: toggle when the setting says so, otherwise just show.
    void toggleRequested();
    void showRequested();

private:
    void buildMenu();
    QMenu* buildDndMenu();
    void reloadBaseImage();
    void rebuildIcon();
    void updateTooltip();
    void applyTrayVisibility();
    void watchSettings();

    core::Settings& m_settings;
    core::ThemeService& m_theme;
    core::DndController& m_dnd;
    Actions& m_actions;
    QSystemTrayIcon* m_tray = nullptr;
    QMenu* m_menu = nullptr;
    QAction* m_dndOff = nullptr;
    QImage m_baseImage;
    QIcon m_icon;
    int m_unread = 0;
    bool m_connected = true;
};

} // namespace whatsie::ui
