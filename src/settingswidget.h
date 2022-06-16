#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include "permissiondialog.h"
#include "utils.h"

#include <QSettings>
#include <QWidget>

namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public QWidget {
  Q_OBJECT

signals:
  void updateWindowTheme();
  void updatePageTheme();
  void muteToggled(const bool checked);
  void autoPlayMediaToggled(const bool checked);
  void userAgentChanged(QString userAgentStr);
  void init_lock();
  void change_lock_password();
  void dictChanged(QString dict);
  void spellCheckChanged(bool checked);
  void notificationPopupTimeOutChanged();
  void notify(QString message);
  void zoomChanged();
  void zoomMaximizedChanged();
  void appAutoLockChanged();

public:
  explicit SettingsWidget(QWidget *parent = nullptr,int screenNumber = 0,
                          QString engineCachePath = "",
                          QString enginePersistentStoragePath = "");
  ~SettingsWidget();

public slots:
  void refresh();
  void updateDefaultUAButton(const QString engineUA);
  void appLockSetChecked(bool checked);
  void setCurrentPasswordText(QString str);
  void loadDictionaries(QStringList dictionaries);

  void clearAllData();
  void autoAppLockSetChecked(bool checked);
  void updateAppLockPasswordViewer();
protected slots:
  bool eventFilter(QObject *obj, QEvent *event);
  void closeEvent(QCloseEvent *event);

private slots:
  QString cachePath();
  QString persistentStoragePath();
  bool isChildOf(QObject *Of, QObject *self);
  void applyThemeQuirks();
  void on_appAutoLockcheckBox_toggled(bool checked);
  void on_applock_checkbox_toggled(bool checked);
  void on_autoLockDurationSpinbox_valueChanged(int arg1);
  void on_autoPlayMediaCheckBox_toggled(bool checked);
  void on_automaticThemeCheckBox_toggled(bool checked);
  void on_changeDefaultDownloadLocationPb_clicked();
  void on_chnageCurrentPasswordPushButton_clicked();
  void on_closeButtonActionComboBox_currentIndexChanged(int index);
  void on_defaultUserAgentButton_clicked();
  void on_deleteCache_clicked();
  void on_deletePersistentData_clicked();
  void on_dictComboBox_currentIndexChanged(const QString &arg1);
  void on_enableSpellCheck_toggled(bool checked);
  void on_minimizeOnTrayIconClick_toggled(bool checked);
  void on_muteAudioCheckBox_toggled(bool checked);
  void on_notificationCheckBox_toggled(bool checked);
  void on_notificationCombo_currentIndexChanged(int index);
  void on_notificationTimeOutspinBox_valueChanged(int arg1);
  void on_resetAppAutoLockPushButton_clicked();
  void on_setUserAgent_clicked();
  void on_showPermissionsButton_clicked();
  void on_showShortcutsButton_clicked();
  void on_startMinimized_toggled(bool checked);
  void on_styleComboBox_currentTextChanged(const QString &arg1);
  void on_themeComboBox_currentTextChanged(const QString &arg1);
  void on_tryNotification_clicked();
  void on_useNativeFileDialog_toggled(bool checked);
  void on_userAgentLineEdit_editingFinished();
  void on_userAgentLineEdit_textChanged(const QString &arg1);
  void on_viewPassword_clicked();
  void on_zoomMinusMaximized_clicked();
  void on_zoomMinus_clicked();
  void on_zoomPlusMaximized_clicked();
  void on_zoomPlus_clicked();
  void on_zoomResetMaximized_clicked();
  void on_zoomReset_clicked();
  void showSetApplockPasswordDialog();
  void themeSwitchTimerTimeout();
  void updateAutomaticTheme();

private:
  Ui::SettingsWidget *ui;
  QString engineCachePath, enginePersistentStoragePath;
  QSettings settings;
  QTimer *themeSwitchTimer;
};

#endif // SETTINGSWIDGET_H
