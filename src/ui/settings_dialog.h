#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QTabWidget;

namespace whatsie::core {
class Settings;
}

namespace whatsie::ui {

/// Paths the storage section reports on and the window may clear.
struct StoragePaths
{
    QString cacheDir;
    QString sessionDir;
};

/// Four tabs, ≤ 25 options total (FEATURES "Settings surface target").
/// Widgets write straight to core::Settings; nothing is buffered, no Apply.
/// Destructive or engine-level operations are signalled to the window.
class SettingsDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(SettingsDialog)

public:
    SettingsDialog(core::Settings& settings, bool trayAvailable, StoragePaths storage,
                   QWidget* parent = nullptr);
    ~SettingsDialog() override = default;

Q_SIGNALS:
    void testNotificationRequested();
    void resetPermissionsRequested();
    void clearCacheRequested();
    void clearSessionRequested();

private:
    void setupUi();
    QWidget* buildGeneralTab();
    QWidget* buildAppearanceTab();
    QWidget* buildNotificationsTab();
    QWidget* buildPrivacyTab();
    void loadValues();
    void refreshStorageSizes();

    core::Settings& m_settings;
    bool m_trayAvailable;
    StoragePaths m_storage;

    QTabWidget* m_tabs = nullptr;
    QComboBox* m_closeAction = nullptr;
    QCheckBox* m_startMinimized = nullptr;
    QCheckBox* m_trayLeftClick = nullptr;
    QLineEdit* m_downloadDir = nullptr;
    QCheckBox* m_askWhereToSave = nullptr;
    QCheckBox* m_showDownloads = nullptr;
    QComboBox* m_theme = nullptr;
    QDoubleSpinBox* m_zoom = nullptr;
    QDoubleSpinBox* m_zoomMaximized = nullptr;
    QCheckBox* m_smoothScrolling = nullptr;
    QCheckBox* m_notificationsEnabled = nullptr;
    QCheckBox* m_notificationSound = nullptr;
    QSpinBox* m_notificationTimeout = nullptr;
    QComboBox* m_hardwareAcceleration = nullptr;
    QLabel* m_cacheSize = nullptr;
    QLabel* m_sessionSize = nullptr;
};

} // namespace whatsie::ui
