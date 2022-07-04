#include "settingswidget.h"
#include "ui_settingswidget.h"

#include "mainwindow.h"
#include <QDateTime>
#include <QMessageBox>
#include <QStyle>

#include "automatictheme.h"

extern QString defaultUserAgentStr;
extern int defaultAppAutoLockDuration;
extern bool defaultAppAutoLock;
extern double defaultZoomFactorMaximized;

SettingsWidget::SettingsWidget(QWidget *parent, int screenNumber,
                               QString engineCachePath,
                               QString enginePersistentStoragePath)
    : QWidget(parent), ui(new Ui::SettingsWidget) {
  ui->setupUi(this);

  this->engineCachePath = engineCachePath;
  this->enginePersistentStoragePath = enginePersistentStoragePath;

  ui->zoomFactorSpinBox->setRange(0.25, 5.0);
  ui->zoomFactorSpinBox->setValue(settings.value("zoomFactor", 1.0).toDouble());

  ui->zoomFactorSpinBoxMaximized->setRange(0.25, 5.0);
  ui->zoomFactorSpinBoxMaximized->setValue(
      settings.value("zoomFactorMaximized", defaultZoomFactorMaximized)
          .toDouble());

  ui->closeButtonActionComboBox->setCurrentIndex(
      settings.value("closeButtonActionCombo", 0).toInt());
  ui->notificationCheckBox->setChecked(
      settings.value("disableNotificationPopups", false).toBool());
  ui->muteAudioCheckBox->setChecked(
      settings.value("muteAudio", false).toBool());
  ui->autoPlayMediaCheckBox->setChecked(
      settings.value("autoPlayMedia", false).toBool());
  ui->themeComboBox->setCurrentText(
      utils::toCamelCase(settings.value("windowTheme", "light").toString()));

  ui->userAgentLineEdit->setText(
      settings.value("useragent", defaultUserAgentStr).toString());
  ui->userAgentLineEdit->home(true);
  ui->userAgentLineEdit->deselect();

  ui->enableSpellCheck->setChecked(settings.value("sc_enabled", true).toBool());
  ui->notificationTimeOutspinBox->setValue(
      settings.value("notificationTimeOut", 9000).toInt() / 1000);
  ui->notificationCombo->setCurrentIndex(
      settings.value("notificationCombo", 1).toInt());
  ui->useNativeFileDialog->setChecked(
      settings.value("useNativeFileDialog", false).toBool());
  ui->startMinimized->setChecked(
      settings.value("startMinimized", false).toBool());

  this->appAutoLockingSetChecked(
      settings.value("appAutoLocking", defaultAppAutoLock).toBool());

  ui->autoLockDurationSpinbox->setValue(
      settings.value("autoLockDuration", defaultAppAutoLockDuration).toInt());
  ui->minimizeOnTrayIconClick->setChecked(
      settings.value("minimizeOnTrayIconClick", false).toBool());
  ui->defaultDownloadLocation->setText(
      settings
          .value("defaultDownloadLocation",
                 QStandardPaths::writableLocation(
                     QStandardPaths::DownloadLocation) +
                     QDir::separator() + QApplication::applicationName())
          .toString());

  ui->styleComboBox->blockSignals(true);
  ui->styleComboBox->addItems(QStyleFactory::keys());
  ui->styleComboBox->blockSignals(false);
  ui->styleComboBox->setCurrentText(
      settings.value("widgetStyle", "Fusion").toString());

  ui->fullWidthViewCheckbox->blockSignals(true);
  ui->fullWidthViewCheckbox->setChecked(
      settings.value("fullWidthView", true).toBool());
  ui->fullWidthViewCheckbox->blockSignals(false);

  ui->automaticThemeCheckBox->blockSignals(true);
  bool automaticThemeSwitching =
      settings.value("automaticTheme", false).toBool();
  ui->automaticThemeCheckBox->setChecked(automaticThemeSwitching);
  ui->automaticThemeCheckBox->blockSignals(false);

  themeSwitchTimer = new QTimer(this);
  themeSwitchTimer->setInterval(60000); // 1 min
  connect(themeSwitchTimer, &QTimer::timeout, &settings,
          [=]() { themeSwitchTimerTimeout(); });

  // instantly call the timeout slot if automatic theme switching enabled
  if (automaticThemeSwitching)
    themeSwitchTimerTimeout();
  // start regular timer to update theme
  updateAutomaticTheme();

  this->setCurrentPasswordText(
      QByteArray::fromBase64(settings.value("asdfg").toString().toUtf8()));

  applyThemeQuirks();

  ui->setUserAgent->setEnabled(false);

  // event filter to prevent wheel event on certain widgets
  foreach (QSlider *slider, this->findChildren<QSlider *>()) {
    slider->installEventFilter(this);
  }
  foreach (QComboBox *box, this->findChildren<QComboBox *>()) {
    box->installEventFilter(this);
  }
  foreach (QSpinBox *spinBox, this->findChildren<QSpinBox *>()) {
    spinBox->installEventFilter(this);
  }

  ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  this->setMinimumHeight(580);

  ui->scrollArea->setMinimumWidth(
      ui->groupBox_8->sizeHint().width() + ui->scrollArea->sizeHint().width() +
      ui->scrollAreaWidgetContents->layout()->spacing());
  if (settings.value("settingsGeo").isValid()) {
    this->restoreGeometry(settings.value("settingsGeo").toByteArray());
    QRect screenRect = QGuiApplication::screens().at(screenNumber)->geometry();
    if (!screenRect.contains(this->pos())) {
      this->move(screenRect.center() - this->rect().center());
    }
  }
}

bool SettingsWidget::eventFilter(QObject *obj, QEvent *event) {

  if (isChildOf(this, obj)) {
    if (event->type() == QEvent::Wheel) {
      return true;
    }
  }
  return QWidget::eventFilter(obj, event);
}

void SettingsWidget::closeEvent(QCloseEvent *event) {
  settings.setValue("settingsGeo", this->saveGeometry());
  QWidget::closeEvent(event);
}

bool SettingsWidget::isChildOf(QObject *Of, QObject *self) {
  bool ischild = false;
  if (Of->findChild<QWidget *>(self->objectName())) {
    ischild = true;
  }
  return ischild;
}

inline bool inRange(unsigned low, unsigned high, unsigned x) {
  return ((x - low) <= (high - low));
}

void SettingsWidget::themeSwitchTimerTimeout() {
  if (settings.value("automaticTheme", false).toBool()) {
    // start time
    QDateTime sunrise;
    sunrise.setSecsSinceEpoch(settings.value("sunrise").toLongLong());
    // end time
    QDateTime sunset;
    sunset.setSecsSinceEpoch(settings.value("sunset").toLongLong());
    QDateTime currentTime = QDateTime::currentDateTime();

    int sunsetSeconds = QTime(0, 0).secsTo(sunset.time());
    int sunriseSeconds = QTime(0, 0).secsTo(sunrise.time());
    int currentSeconds = QTime(0, 0).secsTo(currentTime.time());

    if (inRange(sunsetSeconds, sunriseSeconds, currentSeconds)) {
      qDebug() << "is night: ";
      ui->themeComboBox->setCurrentText("Dark");
    } else {
      qDebug() << "is morn: ";
      ui->themeComboBox->setCurrentText("Light");
    }
  }
}

void SettingsWidget::updateAutomaticTheme() {
  bool automaticThemeSwitching =
      settings.value("automaticTheme", false).toBool();
  if (automaticThemeSwitching && !themeSwitchTimer->isActive()) {
    themeSwitchTimer->start();
  } else if (!automaticThemeSwitching) {
    themeSwitchTimer->stop();
  }
}

SettingsWidget::~SettingsWidget() {
  themeSwitchTimer->stop();
  themeSwitchTimer->deleteLater();
  delete ui;
}

void SettingsWidget::loadDictionaries(QStringList dictionaries) {
  // set up supported spellcheck dictionaries
  QStringList ui_dictionary_names;
  foreach (QString dictionary_name, dictionaries) {
    ui_dictionary_names.append(dictionary_name);
  }

  ui_dictionary_names.removeDuplicates();
  ui_dictionary_names.sort();

  // add to ui
  ui->dictComboBox->blockSignals(true);
  foreach (const QString dict_name, ui_dictionary_names) {
    QString short_name = QString(dict_name).split("_").last();
    short_name = (short_name.isEmpty() || short_name.contains("-"))
                     ? QString(dict_name).split("-").last()
                     : short_name;
    short_name = short_name.isEmpty() ? "XX" : short_name;
    short_name = short_name.length() > 2 ? short_name.left(2) : short_name;
    QIcon icon(QString(":/icons/flags/%1.png").arg(short_name.toLower()));
    if (icon.isNull() == false)
      ui->dictComboBox->addItem(icon, dict_name);
    else
      ui->dictComboBox->addItem(QIcon(":/icons/flags/xx.png"), dict_name);
  }
  ui->dictComboBox->blockSignals(false);

  // load settings for spellcheck dictionary
  QString dictionary_name = settings.value("sc_dict", "en-US").toString();
  int pos = ui->dictComboBox->findText(dictionary_name);
  if (pos == -1) {
    pos = ui->dictComboBox->findText("en-US");
    if (pos == -1) {
      pos = 0;
    }
  }
  ui->dictComboBox->setCurrentIndex(pos);
}

void SettingsWidget::refresh() {
  ui->themeComboBox->setCurrentText(
      utils::toCamelCase(settings.value("windowTheme", "light").toString()));

  ui->cacheSize->setText(utils::refreshCacheSize(cachePath()));
  ui->cookieSize->setText(utils::refreshCacheSize(persistentStoragePath()));

  // update dict settings at runtime
  //  load settings for spellcheck dictionary
  QString dictionary_name = settings.value("sc_dict", "en-US").toString();
  int pos = ui->dictComboBox->findText(dictionary_name);
  if (pos == -1) {
    pos = ui->dictComboBox->findText("en-US");
    if (pos == -1) {
      pos = 0;
    }
  }
  ui->dictComboBox->setCurrentIndex(pos);

  // enable disable spell check
  ui->enableSpellCheck->setChecked(settings.value("sc_enabled", true).toBool());
  emit updateFullWidthView(settings.value("fullWidthView", true).toBool());
}

void SettingsWidget::updateDefaultUAButton(const QString engineUA) {
  bool isDefault =
      QString::compare(engineUA, defaultUserAgentStr, Qt::CaseInsensitive) == 0;
  ui->defaultUserAgentButton->setEnabled(!isDefault);

  if (ui->userAgentLineEdit->text().trimmed().isEmpty()) {
    ui->userAgentLineEdit->setText(engineUA);
  }
}

QString SettingsWidget::cachePath() { return engineCachePath; }

QString SettingsWidget::persistentStoragePath() {
  return enginePersistentStoragePath;
}

void SettingsWidget::on_deleteCache_clicked() {
  QMessageBox msgBox;
  msgBox.setText("This will delete the cache! Cache makes "
                 "application load faster.");
  msgBox.setIconPixmap(
      QPixmap(":/icons/information-line.png")
          .scaled(42, 42, Qt::KeepAspectRatio, Qt::SmoothTransformation));

  msgBox.setInformativeText("Delete cache?");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  int ret = msgBox.exec();
  switch (ret) {
  case QMessageBox::Yes: {
    utils::delete_cache(this->cachePath());
    refresh();
    break;
  }
  case QMessageBox::No:
    break;
  }
}

void SettingsWidget::on_deletePersistentData_clicked() {
  QMessageBox msgBox;
  msgBox.setText("This will delete Persistent Data ! Persistent data includes "
                 "persistent cookies, HTML5 local storage, and visited links.");
  msgBox.setIconPixmap(
      QPixmap(":/icons/information-line.png")
          .scaled(42, 42, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  msgBox.setInformativeText("Delete Cookies?");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  int ret = msgBox.exec();
  switch (ret) {
  case QMessageBox::Yes: {
    clearAllData();
    break;
  }
  case QMessageBox::No:
    break;
  }
}

void SettingsWidget::clearAllData() {
  utils::delete_cache(this->cachePath());
  utils::delete_cache(this->persistentStoragePath());
  refresh();
}

void SettingsWidget::on_notificationCheckBox_toggled(bool checked) {
  settings.setValue("disableNotificationPopups", checked);
}

void SettingsWidget::on_themeComboBox_currentTextChanged(const QString &arg1) {
  applyThemeQuirks();
  settings.setValue("windowTheme", QString(arg1).toLower());
  emit updateWindowTheme();
  emit updatePageTheme();
}

void SettingsWidget::applyThemeQuirks() {
  // little quirks
  if (QString::compare(ui->themeComboBox->currentText(), "dark",
                       Qt::CaseInsensitive) == 0) {
    ui->bottomLine->setStyleSheet("background-color: rgb(0, 117, 96);");
    ui->label_7->setStyleSheet(
        "color:#c2c5d1;padding: 0px 8px 0px 8px;background:transparent;");
    ui->headerWidget->setStyleSheet("background-color: qlineargradient("
                                    "spread:reflect, x1:0, y1:1, x2:1, y2:1,"
                                    "stop:0 rgba(0, 117, 96, 255), "
                                    "stop:0.328358 rgba(0, 117, 96, 144),"
                                    "stop:0.61194 rgba(0, 117, 96, 78),"
                                    "stop:0.895522 rgba(0, 117, 96, 6));");
  } else {
    ui->bottomLine->setStyleSheet("background-color: rgb(37, 211, 102);");
    ui->label_7->setStyleSheet(
        "color:#1e1f21;padding: 0px 8px 0px 8px;background:transparent;");
    ui->headerWidget->setStyleSheet("background-color: qlineargradient("
                                    "spread:reflect, x1:0, y1:1, x2:1, y2:1,"
                                    "stop:0 rgba(37, 211, 102, 200), "
                                    "stop:0.328358 rgba(37, 211, 102, 122),"
                                    "stop:0.61194 rgba(37, 211, 102, 68),"
                                    "stop:0.895522 rgba(37, 211, 102, 6));");
  }
}

void SettingsWidget::on_muteAudioCheckBox_toggled(bool checked) {
  settings.setValue("muteAudio", checked);
  emit muteToggled(checked);
}

void SettingsWidget::on_autoPlayMediaCheckBox_toggled(bool checked) {
  settings.setValue("autoPlayMedia", checked);
  emit autoPlayMediaToggled(checked);
}

void SettingsWidget::on_defaultUserAgentButton_clicked() {
  ui->userAgentLineEdit->setText(defaultUserAgentStr);
  emit userAgentChanged(ui->userAgentLineEdit->text());
}

void SettingsWidget::on_userAgentLineEdit_textChanged(const QString &arg1) {
  bool isDefault = QString::compare(arg1.trimmed(), defaultUserAgentStr,
                                    Qt::CaseInsensitive) == 0;
  bool isPrevious =
      QString::compare(
          arg1.trimmed(),
          settings.value("useragent", defaultUserAgentStr).toString(),
          Qt::CaseInsensitive) == 0;

  if (isDefault == false && arg1.trimmed().isEmpty() == false) {
    ui->defaultUserAgentButton->setEnabled(false);
    ui->setUserAgent->setEnabled(false);
  }
  if (isPrevious == false && arg1.trimmed().isEmpty() == false) {
    ui->setUserAgent->setEnabled(true);
    ui->defaultUserAgentButton->setEnabled(true);
  }
  if (isPrevious) {
    ui->defaultUserAgentButton->setEnabled(true);
    ui->setUserAgent->setEnabled(false);
  }
}

void SettingsWidget::on_setUserAgent_clicked() {
  if (ui->userAgentLineEdit->text().trimmed().isEmpty()) {
    QMessageBox::information(this, QApplication::applicationName() + "| Error",
                             "Cannot set an empty UserAgent String.");
    return;
  }
  emit userAgentChanged(ui->userAgentLineEdit->text());
}

void SettingsWidget::on_closeButtonActionComboBox_currentIndexChanged(
    int index) {
  settings.setValue("closeButtonActionCombo", index);
}

void SettingsWidget::autoAppLockSetChecked(bool checked) {
  ui->appAutoLockcheckBox->blockSignals(true);
  ui->appAutoLockcheckBox->setChecked(checked);
  ui->appAutoLockcheckBox->blockSignals(false);
}

void SettingsWidget::updateAppLockPasswordViewer() {
  this->setCurrentPasswordText(
      QByteArray::fromBase64(settings.value("asdfg").toString().toUtf8()));
}

void SettingsWidget::appLockSetChecked(bool checked) {
  ui->applock_checkbox->blockSignals(true);
  ui->applock_checkbox->setChecked(checked);
  ui->applock_checkbox->blockSignals(false);
}

void SettingsWidget::appAutoLockingSetChecked(bool checked) {
  ui->appAutoLockcheckBox->blockSignals(true);
  ui->appAutoLockcheckBox->setChecked(checked);
  ui->appAutoLockcheckBox->blockSignals(false);
}

void SettingsWidget::toggleTheme() {
  // disable automatic theme first
  if (settings.value("automaticTheme", false).toBool()) {
    emit notify(tr(
        "Automatic theme switching was disabled due to manual theme toggle."));
    ui->automaticThemeCheckBox->setChecked(false);
  }
  if (ui->themeComboBox->currentIndex() == 0) {
    ui->themeComboBox->setCurrentIndex(1);
  } else {
    ui->themeComboBox->setCurrentIndex(0);
  }
}

void SettingsWidget::setCurrentPasswordText(QString str) {
  ui->current_password->setStyleSheet(
      "QLineEdit[echoMode=\"2\"]{lineedit-password-character: 9899}");
  if (str == "Require setup") {
    ui->current_password->setEchoMode(QLineEdit::Normal);
  } else {
    ui->current_password->setEchoMode(QLineEdit::Password);
    ui->current_password->setText(str);
  }
}

void SettingsWidget::on_applock_checkbox_toggled(bool checked) {
  if (settings.value("asdfg").isValid()) {
    settings.setValue("lockscreen", checked);
  } else if (checked && !settings.value("asdfg").isValid()) {
    settings.setValue("lockscreen", true);
    if (checked)
      showSetApplockPasswordDialog();
  } else {
    settings.setValue("lockscreen", false);
    if (checked)
      showSetApplockPasswordDialog();
  }
}

void SettingsWidget::showSetApplockPasswordDialog() {
  QMessageBox msgBox;
  msgBox.setText("App lock is not configured.");
  msgBox.setIconPixmap(
      QPixmap(":/icons/information-line.png")
          .scaled(42, 42, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  msgBox.setInformativeText("Do you want to setup App lock now?");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Yes) {
    this->close();
    emit initLock();
  } else {
    ui->applock_checkbox->blockSignals(true);
    ui->applock_checkbox->setChecked(false);
    ui->applock_checkbox->blockSignals(false);
  }
}

void SettingsWidget::on_dictComboBox_currentIndexChanged(const QString &arg1) {
  settings.setValue("sc_dict", arg1);
  emit dictChanged(arg1);
}

void SettingsWidget::on_enableSpellCheck_toggled(bool checked) {
  settings.setValue("sc_enabled", checked);
  emit spellCheckChanged(checked);
}

void SettingsWidget::on_showShortcutsButton_clicked() {
  QWidget *sheet = new QWidget(this);
  sheet->setWindowTitle(QApplication::applicationName() +
                        " | Global shortcuts");

  sheet->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
  sheet->move(this->geometry().center() - sheet->geometry().center());

  QVBoxLayout *layout = new QVBoxLayout(sheet);
  sheet->setLayout(layout);
  auto *w = qobject_cast<MainWindow *>(parent());
  if (w != 0) {
    foreach (QAction *action, w->actions()) {
      QString shortcutStr = action->shortcut().toString();
      if (shortcutStr.isEmpty() == false) {
        QLabel *label = new QLabel(
            action->text().remove("&") + "  |  " + shortcutStr, sheet);
        label->setAlignment(Qt::AlignHCenter);
        layout->addWidget(label);
      }
      // handle special case for minimize to try action
      if (action->text().contains("minimize", Qt::CaseInsensitive) ||
          action->text().contains("Mi&nimize to tray")) {
        QLabel *label =
            new QLabel(action->text().remove("&") + "  |  " + "Ctrl+W", sheet);
        label->setAlignment(Qt::AlignHCenter);
        layout->addWidget(label);
      }
    }
  }
  sheet->setAttribute(Qt::WA_DeleteOnClose);
  sheet->show();
}

void SettingsWidget::on_showPermissionsButton_clicked() {
  PermissionDialog *permissionDialog = new PermissionDialog(this);
  permissionDialog->setWindowTitle(QApplication::applicationName() + " | " +
                                   tr("Feature permissions"));
  permissionDialog->setWindowFlag(Qt::Dialog);
  permissionDialog->setAttribute(Qt::WA_DeleteOnClose, true);
  permissionDialog->move(this->geometry().center() -
                         permissionDialog->geometry().center());
  permissionDialog->setMinimumSize(485, 310);
  permissionDialog->adjustSize();
  permissionDialog->show();
}

void SettingsWidget::on_notificationTimeOutspinBox_valueChanged(int arg1) {
  settings.setValue("notificationTimeOut", arg1 * 1000);
  emit notificationPopupTimeOutChanged();
}

void SettingsWidget::on_notificationCombo_currentIndexChanged(int index) {
  settings.setValue("notificationCombo", index);
}

void SettingsWidget::on_tryNotification_clicked() {
  emit notify("This is a long long long test notification......");
}

void SettingsWidget::on_automaticThemeCheckBox_toggled(bool checked) {
  if (checked) {
    AutomaticTheme *automaticTheme = new AutomaticTheme(this);
    automaticTheme->setWindowTitle(QApplication::applicationName() +
                                   " | Automatic theme switcher setup");
    automaticTheme->setWindowFlag(Qt::Dialog);
    automaticTheme->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(automaticTheme, &AutomaticTheme::destroyed,
            ui->automaticThemeCheckBox, [=]() {
              bool automaticThemeSwitching =
                  settings.value("automaticTheme", false).toBool();
              ui->automaticThemeCheckBox->setChecked(automaticThemeSwitching);
              if (automaticThemeSwitching)
                themeSwitchTimerTimeout();
              updateAutomaticTheme();
            });
    automaticTheme->show();
  } else {
    settings.setValue("automaticTheme", false);
    updateAutomaticTheme();
  }
}

void SettingsWidget::on_useNativeFileDialog_toggled(bool checked) {
  settings.setValue("useNativeFileDialog", checked);
}

void SettingsWidget::on_startMinimized_toggled(bool checked) {
  settings.setValue("startMinimized", checked);
}

void SettingsWidget::on_appAutoLockcheckBox_toggled(bool checked) {
  if (settings.value("asdfg").isValid()) {
    settings.setValue("appAutoLocking", checked);
  } else {
    QMessageBox::information(this, "App Lock Setup",
                             "Please setup the App lock password first.",
                             QMessageBox::Ok);
    if (settings.value("asdfg").isValid() == false) {
      settings.setValue("appAutoLocking", false);
      autoAppLockSetChecked(false);
    }
  }
  emit appAutoLockChanged();
}

void SettingsWidget::on_autoLockDurationSpinbox_valueChanged(int arg1) {
  settings.setValue("autoLockDuration", arg1);
  emit appAutoLockChanged();
}

void SettingsWidget::on_resetAppAutoLockPushButton_clicked() {
  ui->appAutoLockcheckBox->setChecked(defaultAppAutoLock);
  ui->autoLockDurationSpinbox->setValue(defaultAppAutoLockDuration);
}

void SettingsWidget::on_minimizeOnTrayIconClick_toggled(bool checked) {
  settings.setValue("minimizeOnTrayIconClick", checked);
}

void SettingsWidget::on_styleComboBox_currentTextChanged(const QString &arg1) {
  applyThemeQuirks();
  settings.setValue("widgetStyle", arg1);
  emit updateWindowTheme();
  emit updatePageTheme();
}

void SettingsWidget::on_zoomPlus_clicked() {
  double currentFactor = settings.value("zoomFactor", 1.0).toDouble();
  double newFactor = currentFactor + 0.25;
  ui->zoomFactorSpinBox->setValue(newFactor);
  settings.setValue("zoomFactor", ui->zoomFactorSpinBox->value());
  emit zoomChanged();
}

void SettingsWidget::on_zoomMinus_clicked() {
  double currentFactor = settings.value("zoomFactor", 1.0).toDouble();
  double newFactor = currentFactor - 0.25;
  ui->zoomFactorSpinBox->setValue(newFactor);
  settings.setValue("zoomFactor", ui->zoomFactorSpinBox->value());
  emit zoomChanged();
}

void SettingsWidget::on_zoomReset_clicked() {
  ui->zoomFactorSpinBox->setValue(1.0);
  settings.setValue("zoomFactor", ui->zoomFactorSpinBox->value());
  emit zoomChanged();
}

void SettingsWidget::on_zoomResetMaximized_clicked() {
  ui->zoomFactorSpinBoxMaximized->setValue(defaultZoomFactorMaximized);
  settings.setValue("zoomFactorMaximized",
                    ui->zoomFactorSpinBoxMaximized->value());
  emit zoomMaximizedChanged();
}

void SettingsWidget::on_zoomPlusMaximized_clicked() {
  double currentFactor =
      settings.value("zoomFactorMaximized", defaultZoomFactorMaximized)
          .toDouble();
  double newFactor = currentFactor + 0.25;
  ui->zoomFactorSpinBoxMaximized->setValue(newFactor);
  settings.setValue("zoomFactorMaximized",
                    ui->zoomFactorSpinBoxMaximized->value());
  emit zoomMaximizedChanged();
}

void SettingsWidget::on_zoomMinusMaximized_clicked() {
  double currentFactor =
      settings.value("zoomFactorMaximized", defaultZoomFactorMaximized)
          .toDouble();
  double newFactor = currentFactor - 0.25;
  ui->zoomFactorSpinBoxMaximized->setValue(newFactor);
  settings.setValue("zoomFactorMaximized",
                    ui->zoomFactorSpinBoxMaximized->value());
  emit zoomMaximizedChanged();
}

void SettingsWidget::on_changeDefaultDownloadLocationPb_clicked() {
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::Directory);
  dialog.setOption(QFileDialog::ShowDirsOnly);

  QString path;
  bool usenativeFileDialog =
      settings.value("useNativeFileDialog", false).toBool();
  if (usenativeFileDialog == false) {
    path = QFileDialog::getExistingDirectory(
        this, tr("Select download directory"),
        settings
            .value("defaultDownloadLocation",
                   QStandardPaths::writableLocation(
                       QStandardPaths::DownloadLocation) +
                       QDir::separator() + QApplication::applicationName())
            .toString(),
        QFileDialog::DontUseNativeDialog);
  } else {
    path = QFileDialog::getSaveFileName(
        this, tr("Select download directory"),
        settings
            .value("defaultDownloadLocation",
                   QStandardPaths::writableLocation(
                       QStandardPaths::DownloadLocation) +
                       QDir::separator() + QApplication::applicationName())
            .toString());
  }

  if (!path.isNull() && !path.isEmpty()) {
    ui->defaultDownloadLocation->setText(path);
    settings.setValue("defaultDownloadLocation", path);
  }
}

void SettingsWidget::on_userAgentLineEdit_editingFinished() {
  ui->userAgentLineEdit->home(true);
  ui->userAgentLineEdit->deselect();
}

void SettingsWidget::on_viewPassword_clicked() {
  ui->current_password->setEchoMode(QLineEdit::Normal);
  ui->viewPassword->setEnabled(false);
  QTimer *timer = new QTimer(this);
  timer->setInterval(5000);
  connect(timer, &QTimer::timeout, ui->current_password, [=]() {
    ui->current_password->setEchoMode(QLineEdit::Password);
    ui->viewPassword->setEnabled(true);
    timer->stop();
    timer->deleteLater();
  });
  timer->start();
}

void SettingsWidget::on_chnageCurrentPasswordPushButton_clicked() {
  if (settings.value("asdfg").isValid()) {
    QMessageBox msgBox;
    msgBox.setText("You are about to change your current app lock password!"
                   "\n\nThis will LogOut your current session."
                   "\nYou may also require a complete restart of Application!");
    msgBox.setIconPixmap(
        QPixmap(":/icons/information-line.png")
            .scaled(42, 42, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    msgBox.setInformativeText("Do you want to proceed?");
    msgBox.setStandardButtons(QMessageBox::Cancel);
    QPushButton *changePassword =
        new QPushButton(this->style()->standardIcon(QStyle::SP_DialogYesButton),
                        "Change password", nullptr);
    msgBox.addButton(changePassword, QMessageBox::NoRole);
    connect(changePassword, &QPushButton::clicked, changePassword, [=]() {
      this->close();
      emit changeLockPassword();
    });
    msgBox.exec();

  } else {
    settings.setValue("lockscreen", true);
    showSetApplockPasswordDialog();
  }
}

void SettingsWidget::on_fullWidthViewCheckbox_toggled(bool checked) {
  settings.setValue("fullWidthView", checked);
  emit updateFullWidthView(checked);
}
