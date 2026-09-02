#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QTabWidget;
class QWebEngineProfile;

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

/// Five tabs, ≤ 25 options total (FEATURES "Settings surface target").
/// Widgets write straight to core::Settings; nothing is buffered, no Apply.
/// Destructive or engine-level operations are signalled to the window.
class SettingsDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(SettingsDialog)

public:
    SettingsDialog(core::Settings& settings, bool trayAvailable, StoragePaths storage,
                   QWebEngineProfile& profile, QWidget* parent = nullptr);
    ~SettingsDialog() override = default;

Q_SIGNALS:
    void testNotificationRequested();
    void clearCacheRequested();
    void clearSessionRequested();

private:
    void setupUi();
    QWidget* wrapInScroll(QWidget* content);
    QWidget* buildGeneralTab();
    QWidget* buildAppearanceTab();
    QWidget* buildNotificationsTab();
    QWidget* buildPrivacyTab();
    QWidget* buildAdvancedTab();
    void loadValues();
    void refreshStorageSizes();
    QWidget* buildNetworkGroup();
    QWidget* buildLockGroup();
    void updateProxyEnabled();
    void updateLockUi();

    core::Settings& m_settings;
    bool m_trayAvailable;
    StoragePaths m_storage;
    QWebEngineProfile& m_profile;

    QTabWidget* m_tabs = nullptr;
    QComboBox* m_closeAction = nullptr;
    QCheckBox* m_startMinimized = nullptr;
    QCheckBox* m_trayLeftClick = nullptr;
    QCheckBox* m_autostart = nullptr;
    QCheckBox* m_traySymbolic = nullptr;
    QCheckBox* m_trayHidden = nullptr;
    QCheckBox* m_trayDim = nullptr;
    QLineEdit* m_downloadDir = nullptr;
    QCheckBox* m_askWhereToSave = nullptr;
    QCheckBox* m_showDownloads = nullptr;
    QComboBox* m_theme = nullptr;
    QDoubleSpinBox* m_zoom = nullptr;
    QDoubleSpinBox* m_zoomMaximized = nullptr;
    QCheckBox* m_smoothScrolling = nullptr;
    QDoubleSpinBox* m_interfaceScale = nullptr;
    QComboBox* m_messageBlur = nullptr;
    QCheckBox* m_notificationsEnabled = nullptr;
    QCheckBox* m_notificationSound = nullptr;
    QSpinBox* m_notificationTimeout = nullptr;
    QComboBox* m_hardwareAcceleration = nullptr;
    QCheckBox* m_spellCheck = nullptr;
    QComboBox* m_spellLanguage = nullptr;
    QLabel* m_lockStatus = nullptr;
    QPushButton* m_lockSet = nullptr;
    QPushButton* m_lockRemove = nullptr;
    QCheckBox* m_lockOnStart = nullptr;
    QCheckBox* m_lockOnHide = nullptr;
    QSpinBox* m_lockIdle = nullptr;
    QCheckBox* m_webrtcPublicOnly = nullptr;
    QComboBox* m_proxyMode = nullptr;
    QComboBox* m_proxyType = nullptr;
    QLineEdit* m_proxyHost = nullptr;
    QSpinBox* m_proxyPort = nullptr;
    QLineEdit* m_proxyUser = nullptr;
    QLineEdit* m_proxyPassword = nullptr;
    QLabel* m_cacheSize = nullptr;
    QLabel* m_sessionSize = nullptr;
};

} // namespace whatsie::ui
