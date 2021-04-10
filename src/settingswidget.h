#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>
#include "utils.h"

#include "permissiondialog.h"



namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public QWidget
{
    Q_OBJECT

signals:
    void updateWindowTheme();
    void updatePageTheme();
    void muteToggled(const bool checked);
    void autoPlayMediaToggled(const bool checked);
    void userAgentChanged(QString userAgentStr);
    void init_lock();
    void dictChanged(QString dict);
    void spellCheckChanged(bool checked);
public:
    explicit SettingsWidget(QWidget *parent = nullptr,QString engineCachePath = "",
                            QString enginePersistentStoragePath = "");
    ~SettingsWidget();

public slots:
    void refresh();
    void updateDefaultUAButton(const QString engineUA);
    void appLockSetChecked(bool checked);
    void setCurrentPasswordText(QString str);
    void loadDictionaries(QStringList dictionaries);
private slots:
    QString cachePath();
    QString persistentStoragePath();

    void on_deleteCache_clicked();

    void on_deletePersistentData_clicked();

    void on_notificationCheckBox_toggled(bool checked);

    void on_themeComboBox_currentTextChanged(const QString &arg1);

    void applyThemeQuirks();
    void on_muteAudioCheckBox_toggled(bool checked);

    void on_defaultUserAgentButton_clicked();

    void on_userAgentLineEdit_textChanged(const QString &arg1);

    void on_setUserAgent_clicked();

    void on_autoPlayMediaCheckBox_toggled(bool checked);

    void on_closeButtonActionComboBox_currentIndexChanged(int index);

    void on_applock_checkbox_toggled(bool checked);

    void on_dictComboBox_currentIndexChanged(const QString &arg1);

    void on_enableSpellCheck_toggled(bool checked);

    void on_showShortcutsButton_clicked();

    void on_showPermissionsButton_clicked();


private:
    Ui::SettingsWidget *ui;
    QString engineCachePath,enginePersistentStoragePath;
    QSettings settings;
};

#endif // SETTINGSWIDGET_H
