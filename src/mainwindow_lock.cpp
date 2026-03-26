// App lock, auto-lock, and session-logout helpers.
#include "mainwindow.h"

#include "common.h"

// ── Lock initialisation ───────────────────────────────────────────────────────

void MainWindow::initAutoLock() {
  m_autoLockEventFilter = new AutoLockEventFilter(
      SettingsManager::instance()
          .settings()
          .value("autoLockDuration", defaultAppAutoLockDuration)
          .toInt() *
      1000);
  connect(m_autoLockEventFilter, &AutoLockEventFilter::autoLockTimerTimeout,
          this, [=]() {
            if ((m_settingsWidget && !m_settingsWidget->isVisible()) &&
                SettingsManager::instance()
                    .settings()
                    .value("appAutoLocking", defaultAppAutoLock)
                    .toBool()) {
              this->lockApp();
            }
          });
  if (SettingsManager::instance()
          .settings()
          .value("appAutoLocking", defaultAppAutoLock)
          .toBool()) {
    qApp->installEventFilter(m_autoLockEventFilter);
  }
}

void MainWindow::initLock() {
  if (m_lockWidget == nullptr) {
    m_lockWidget = new Lock(this);
    m_lockWidget->setObjectName("lockWidget");
    m_lockWidget->setWindowFlags(Qt::Widget);
    m_lockWidget->setStyleSheet(
        "QWidget#login{background-color:palette(window)};"
        "QWidget#signup{background-color:palette(window)}");
    m_lockWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(m_lockWidget, &Lock::passwordNotSet, m_settingsWidget, [=]() {
      SettingsManager::instance().settings().setValue("lockscreen", false);
      m_settingsWidget->appLockSetChecked(false);
    });

    connect(m_lockWidget, &Lock::unLocked, [=]() {
      // unlock event
    });

    connect(m_lockWidget, &Lock::passwordSet, m_settingsWidget, [=]() {
      if (SettingsManager::instance().settings().value("asdfg").isValid()) {
        m_settingsWidget->setCurrentPasswordText(
            QByteArray::fromBase64(SettingsManager::instance()
                                       .settings()
                                       .value("asdfg")
                                       .toString()
                                       .toUtf8()));
      } else {
        m_settingsWidget->setCurrentPasswordText("Require setup");
      }
      m_settingsWidget->appLockSetChecked(SettingsManager::instance()
                                              .settings()
                                              .value("lockscreen", false)
                                              .toBool());
    });
    m_lockWidget->applyThemeQuirks();
  }

  m_lockWidget->setGeometry(this->rect());
  QApplication::processEvents();

  if (SettingsManager::instance().settings().value("lockscreen").toBool()) {
    if (SettingsManager::instance().settings().value("asdfg").isValid()) {
      m_lockWidget->lock_app();
    } else {
      m_lockWidget->signUp();
    }
    m_lockWidget->show();
  } else {
    m_lockWidget->hide();
  }
  updateWindowTheme();
}

void MainWindow::tryLock() {
  if (SettingsManager::instance().settings().value("asdfg").isValid()) {
    initLock();
    return;
  }
  // No password configured — reset lock-related settings.
  SettingsManager::instance().settings().setValue("lockscreen", false);
  SettingsManager::instance().settings().setValue("appAutoLocking", false);
  m_settingsWidget->appAutoLockingSetChecked(false);
  m_settingsWidget->appLockSetChecked(false);
  initLock();
}

// ── Lock/unlock actions ───────────────────────────────────────────────────────

void MainWindow::lockApp() {
  if (m_lockWidget != nullptr && m_lockWidget->getIsLocked())
    return;

  if (SettingsManager::instance().settings().value("asdfg").isValid()) {
    initLock();
    m_lockWidget->lock_app();
  } else {
    int ret = QMessageBox::information(
        this, tr(QApplication::applicationName().toUtf8()),
        tr("App lock is not configured, \n"
           "Please setup the password in the Settings first.\n\nOpen "
           "Settings now?"),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Yes)
      this->showSettings();
  }
}

void MainWindow::changeLockPassword() {
  SettingsManager::instance().settings().remove("asdfg");
  m_settingsWidget->appLockSetChecked(false);
  m_settingsWidget->autoAppLockSetChecked(false);
  m_settingsWidget->updateAppLockPasswordViewer();
  tryLogOut();
  QTimer::singleShot(1000, this, [=]() {
    if (isLoggedIn()) {
      forceLogOut();
      doAppReload();
    }
    appAutoLockChanged();
    initLock();
  });
}

void MainWindow::appAutoLockChanged() {
  bool enabled = SettingsManager::instance()
                     .settings()
                     .value("appAutoLocking", defaultAppAutoLock)
                     .toBool();
  if (enabled) {
    m_autoLockEventFilter->setTimeoutmillis(
        SettingsManager::instance()
            .settings()
            .value("autoLockDuration", defaultAppAutoLockDuration)
            .toInt() *
        1000);
    qApp->installEventFilter(m_autoLockEventFilter);
    m_autoLockEventFilter->resetTimer();
  } else {
    m_autoLockEventFilter->stopTimer();
    qApp->removeEventFilter(m_autoLockEventFilter);
  }
}

// ── Session helpers ───────────────────────────────────────────────────────────

void MainWindow::forceLogOut() {
  if (m_webEngine && m_webEngine->page()) {
    m_webEngine->page()->runJavaScript(
        "window.localStorage.clear();",
        [=](const QVariant &result) { qDebug() << result; });
  }
}

bool MainWindow::isLoggedIn() {
  static bool loggedIn = false;
  if (m_webEngine && m_webEngine->page()) {
    m_webEngine->page()->runJavaScript(
        "window.localStorage.getItem('last-wid-md')",
        [=](const QVariant &result) {
          qDebug() << Q_FUNC_INFO << result;
          if (result.isValid() && !result.toString().isEmpty())
            loggedIn = true;
        });
    qDebug() << "isLoggedIn" << loggedIn;
    return loggedIn;
  }
  qDebug() << "isLoggedIn" << loggedIn;
  return loggedIn;
}

void MainWindow::tryLogOut() {
  if (m_webEngine && m_webEngine->page()) {
    m_webEngine->page()->runJavaScript(
        "document.querySelector(\"span[data-testid|='menu']\").click();"
        "document.querySelector(\"#side > header > div > div > span > div > "
        "span > div > ul > li:nth-child(4) > div\").click();"
        "var dialogEle,dialogEleLastElem;"
        "function logoutC(){"
        "  dialogEle=document.activeElement.querySelectorAll(\":last-child\");"
        "  dialogEleLastElem=dialogEle[dialogEle.length-1];"
        "  dialogEleLastElem.click();"
        "}"
        "setTimeout(logoutC, 600);",
        [=](const QVariant &result) { qDebug() << Q_FUNC_INFO << result; });
  }
}
