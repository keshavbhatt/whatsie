#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QTabWidget;

namespace whatsie::core {
class Settings;
}

namespace whatsie::ui {

/// Four tabs, ≤ 25 options total (FEATURES "Settings surface target").
/// Widgets write straight to core::Settings; nothing is buffered, no Apply.
class SettingsDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(SettingsDialog)

public:
    SettingsDialog(core::Settings& settings, bool trayAvailable, QWidget* parent = nullptr);
    ~SettingsDialog() override = default;

private:
    void setupUi();
    QWidget* buildGeneralTab();
    QWidget* buildAppearanceTab();
    QWidget* buildNotificationsTab();
    QWidget* buildAdvancedTab();
    void loadValues();

    core::Settings& m_settings;
    bool m_trayAvailable;

    QTabWidget* m_tabs = nullptr;
    QComboBox* m_closeAction = nullptr;
    QCheckBox* m_startMinimized = nullptr;
    QCheckBox* m_trayLeftClick = nullptr;
    QComboBox* m_theme = nullptr;
    QDoubleSpinBox* m_zoom = nullptr;
    QDoubleSpinBox* m_zoomMaximized = nullptr;
    QCheckBox* m_smoothScrolling = nullptr;
};

} // namespace whatsie::ui
