// WebEngine page/profile lifecycle, reload, download, and page-theme handling.
#include "mainwindow.h"

#include <QRandomGenerator>
#include <QScreen>
#include <QWebEngineNotification>

#include "common.h"
#include "dictionaries.h"
#include "webengineprofilemanager.h"
#include "webview.h"

// ── WebEngine view & page ─────────────────────────────────────────────────────

void MainWindow::createWebEngine() {
  WebEngineProfileManager::instance().applyUserSettings();

  QSizePolicy widgetSize;
  widgetSize.setHorizontalPolicy(QSizePolicy::Expanding);
  widgetSize.setVerticalPolicy(QSizePolicy::Expanding);
  widgetSize.setHorizontalStretch(1);
  widgetSize.setVerticalStretch(1);

  m_dictionaries = Dictionaries::GetDictionaries();

  WebView *webEngineView = new WebView(this, m_dictionaries);
  setCentralWidget(webEngineView);
  webEngineView->setSizePolicy(widgetSize);
  webEngineView->show();

  m_webEngine = webEngineView;
  webEngineView->addAction(m_minimizeAction);
  webEngineView->addAction(m_lockAction);
  webEngineView->addAction(m_quitAction);

  createWebPage(false);
}

void MainWindow::createWebPage(bool offTheRecord) {
  QWebEngineProfile *profile = nullptr;

  if (offTheRecord) {
    if (!m_otrProfile)
      m_otrProfile.reset(new QWebEngineProfile);
    profile = m_otrProfile.get();
  } else {
    profile = WebEngineProfileManager::instance().profile();
    WebEngineProfileManager::instance().applyUserSettings();
  }

  setNotificationPresenter(profile);

  QWebEnginePage *page = new WebEnginePage(profile, m_webEngine);
  if (SettingsManager::instance()
          .settings()
          .value("windowTheme", "light")
          .toString() == "dark") {
    page->setBackgroundColor(QColor(17, 27, 33));   // WhatsApp dark bg
  } else {
    page->setBackgroundColor(QColor(240, 240, 240)); // WhatsApp light bg
  }
  m_webEngine->setPage(page);

  if (offTheRecord) {
    // Transfer ownership so the OTR profile is cleaned up with the page.
    profile->setParent(page);
  }

  auto randomValue = QRandomGenerator::global()->generateDouble() * 300.0;
  page->setUrl(
      QUrl("https://web.whatsapp.com?v=" + QString::number(randomValue)));

  connect(profile, &QWebEngineProfile::downloadRequested,
          &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested);
  connect(page, &QWebEnginePage::fullScreenRequested, this,
          &MainWindow::fullScreenRequested);

  double currentFactor = SettingsManager::instance()
                             .settings()
                             .value("zoomFactor", 1.0)
                             .toDouble();
  m_webEngine->page()->setZoomFactor(currentFactor);
}

void MainWindow::setNotificationPresenter(QWebEngineProfile *profile) {
  if (m_webengine_notifier_popup != nullptr) {
    m_webengine_notifier_popup->close();
    m_webengine_notifier_popup->deleteLater();
  }

  m_webengine_notifier_popup = new NotificationPopup(m_webEngine);
  connect(m_webengine_notifier_popup, &NotificationPopup::notification_clicked,
          this, [this]() { notificationClicked(); });

  profile->setNotificationPresenter(
      [&](std::unique_ptr<QWebEngineNotification> notification) {
        QSettings &settings = SettingsManager::instance().settings();
        if (settings.value("disableNotificationPopups", false).toBool())
          return;

        int notificationCombo = settings.value("notificationCombo", 1).toInt();
        int timeout = settings.value("notificationTimeOut", 9000).toInt();

        if (notificationCombo == 0 && m_systemTrayIcon) {
          QPointer<QWebEngineNotification> notificationPtr = notification.get();
          if (notificationPtr) {
            connect(m_systemTrayIcon, &QSystemTrayIcon::messageClicked, this,
                    [this, notificationPtr]() {
                      QMetaObject::invokeMethod(
                          this,
                          [notificationPtr]() {
                            if (notificationPtr) {
                              qWarning() << "notificationPtr clciked";
                              notificationPtr->click();
                            }
                            qWarning() << "notificationPtr clciked Ok";
                          },
                          Qt::QueuedConnection);
                    });
          }

          // Custom icons cause issues on GNOME Shell.
          if (userDesktopEnvironment.contains("gnome", Qt::CaseInsensitive)) {
            m_systemTrayIcon->showMessage(notification->title(),
                                          notification->message(),
                                          QSystemTrayIcon::Critical, 0);
          } else {
            QIcon icon(QPixmap::fromImage(notification->icon()));
            m_systemTrayIcon->showMessage(
                notification->title(), notification->message(), icon, timeout);
          }
          return;
        }

        if (!m_webengine_notifier_popup) {
          qWarning() << "Popup is not available!";
          return;
        }

        m_webengine_notifier_popup->setMinimumWidth(300);
        QScreen *screen = QGuiApplication::primaryScreen();
        if (!screen) {
          const auto screens = QGuiApplication::screens();
          if (!screens.isEmpty()) {
            screen = screens.first();
          } else {
            qWarning() << "showNotification: unable to get any screen";
            return;
          }
        }
        m_webengine_notifier_popup->present(screen, notification);
      });
}

// ── Reload & load events ──────────────────────────────────────────────────────

void MainWindow::doAppReload() {
  if (m_webEngine->page())
    m_webEngine->page()->disconnect();
  createWebPage(false);
}

void MainWindow::doReload(bool byPassCache, bool isAskedByCLI,
                          bool byLoadingQuirk) {
  if (byLoadingQuirk) {
    m_webEngine->triggerPageAction(QWebEnginePage::ReloadAndBypassCache,
                                   byPassCache);
    return;
  }

  if (m_lockWidget && !m_lockWidget->getIsLocked()) {
    this->showNotification(QApplication::applicationName(),
                           QObject::tr("Reloading..."));
  } else {
    QString error = tr("Unlock to Reload the App.");
    if (isAskedByCLI) {
      this->showNotification(QApplication::applicationName() + "| Error",
                             error);
    } else {
      QMessageBox::critical(this, QApplication::applicationName() + "| Error",
                            error);
    }
    this->show();
    return;
  }
  m_webEngine->triggerPageAction(QWebEnginePage::ReloadAndBypassCache,
                                 byPassCache);
}

void MainWindow::handleLoadFinished(bool loaded) {
  if (loaded) {
    qDebug() << "Loaded";
    checkLoadedCorrectly();
    updatePageTheme();
    handleZoom();
    if (m_settingsWidget != nullptr)
      m_settingsWidget->refresh();
  }
}

void MainWindow::checkLoadedCorrectly() {
  if (!m_webEngine || !m_webEngine->page())
    return;

  m_webEngine->page()->runJavaScript(
      "document.querySelector('body').className",
      [this](const QVariant &result) {
        if (result.toString().contains("page-version", Qt::CaseInsensitive)) {
          qDebug() << "Test 1 found" << result.toString();
          m_webEngine->page()->runJavaScript(
              "document.getElementsByTagName('body')[0].innerText = ''");
          loadingQuirk("test1");
        } else if (m_webEngine->title().contains("Error",
                                                 Qt::CaseInsensitive)) {
          Utils::delete_cache(m_webEngine->page()->profile()->cachePath());
          Utils::delete_cache(
              m_webEngine->page()->profile()->persistentStoragePath());
          SettingsManager::instance().settings().setValue("useragent",
                                                          defaultUserAgentStr);
          Utils::DisplayExceptionErrorDialog(
              "test1 handleWebViewTitleChanged(title) title: Error, "
              "Resetting UA, Quiting!\nUA: " +
              SettingsManager::instance()
                  .settings()
                  .value("useragent", "DefaultUA")
                  .toString());
          m_quitAction->trigger();
        } else {
          qDebug() << "Test 1 loaded correctly, value:" << result.toString();
        }
      });
}

void MainWindow::loadingQuirk(const QString &test) {
  if (m_correctlyLoadedRetries > -1) {
    qWarning() << test << "checkLoadedCorrectly()/loadingQuirk()/doReload()"
               << m_correctlyLoadedRetries;
    doReload(false, false, true);
    m_correctlyLoadedRetries--;
  } else {
    Utils::delete_cache(m_webEngine->page()->profile()->cachePath());
    Utils::delete_cache(
        m_webEngine->page()->profile()->persistentStoragePath());
    SettingsManager::instance().settings().setValue("useragent",
                                                    defaultUserAgentStr);
    Utils::DisplayExceptionErrorDialog(
        test +
        " checkLoadedCorrectly()/loadingQuirk() reload retries 0, Resetting "
        "UA, Quiting!\nUA: " +
        SettingsManager::instance()
            .settings()
            .value("useragent", "DefaultUA")
            .toString());
    m_quitAction->trigger();
  }
}

// ── Page theme ────────────────────────────────────────────────────────────────

void MainWindow::updatePageTheme() {
  if (!m_webEngine || !m_webEngine->page())
    return;

  const bool dark = SettingsManager::instance()
                        .settings()
                        .value("windowTheme", "light")
                        .toString() == "dark";

  // Sequence reverse-engineered from WhatsApp Web's own theme-toggle logic:
  //
  // 1. WA module calls via global require():
  //      WAWebUserPrefsGeneral.setSystemThemeMode(false)  -- disable "follow OS"
  //      WAWebUserPrefsGeneral.setTheme(theme)            -- persist preference
  //      WAWebThemeContext.applyThemeToUI(theme)          -- repaint UI
  //      WAWebSystemTheme.theme = theme                   -- update system ref
  //
  // 2. React class component setState -- walk fiber ancestors of .app-wrapper-web
  //    upward via .return until we find the component whose state has both
  //    .theme and .systemThemeMode, then setState({theme, systemThemeMode:false}).
  //    Fall back to forceUpdate() on ancestors if that component is not found.
  //
  // 3. DOM attributes + localStorage -- persists across reloads, covers
  //    any CSS-only observers.
  const QString js = QString(R"js(
    (function(theme) {
      var isDark = theme === 'dark';

      // ── 1. WA module calls via global require() ───────────────────────────
      if (typeof require === 'function') {
        try {
          var up = require('WAWebUserPrefsGeneral');
          if (up) {
            if (typeof up.setSystemThemeMode === 'function') up.setSystemThemeMode(false);
            if (typeof up.setTheme          === 'function') up.setTheme(theme);
          }
        } catch (e) {}
        try {
          var tc = require('WAWebThemeContext');
          if (tc && typeof tc.applyThemeToUI === 'function') tc.applyThemeToUI(theme);
        } catch (e) {}
        try {
          var st = require('WAWebSystemTheme');
          if (st) st.theme = theme;
        } catch (e) {}
      }

      // ── 2. React class component setState (upward fiber walk) ─────────────
      try {
        var wrapper = document.querySelector('.app-wrapper-web');
        var rk = wrapper && Object.keys(wrapper).find(function(k) {
          return k.startsWith('__reactFiber') || k.startsWith('__reactInternalInstance');
        });
        if (rk) {
          var fiber = wrapper[rk];
          var found = false;
          while (fiber) {
            var sn = fiber.stateNode;
            if (sn && sn.state &&
                sn.state.theme !== undefined &&
                sn.state.systemThemeMode !== undefined &&
                typeof sn.setState === 'function') {
              sn.setState({theme: theme, systemThemeMode: false});
              found = true;
              break;
            }
            fiber = fiber.return;
          }
          if (!found) {
            fiber = wrapper[rk];
            var count = 0;
            while (fiber && count < 10) {
              if (fiber.stateNode && typeof fiber.stateNode.forceUpdate === 'function') {
                try { fiber.stateNode.forceUpdate(); } catch (e) {}
                count++;
              }
              fiber = fiber.return;
            }
          }
        }
      } catch (e) {}

      // ── 3. DOM attributes + localStorage ─────────────────────────────────
      var root = document.documentElement;
      root.setAttribute('data-theme',      theme);
      root.setAttribute('data-color-mode', theme);
      root.style.colorScheme = theme;
      document.body.classList.toggle('dark', isDark);

      localStorage.setItem('theme', theme);
      localStorage.removeItem('system-theme-mode');
      try {
        window.dispatchEvent(new StorageEvent('storage', {
          key: 'theme', newValue: theme,
          storageArea: localStorage, url: location.href
        }));
      } catch (e) {}
    })('%1');
  )js").arg(dark ? "dark" : "light");

  m_webEngine->page()->runJavaScript(js);
}

QString MainWindow::getPageTheme() const {
  static QString theme = "web"; // implies light
  if (m_webEngine && m_webEngine->page()) {
    // Read back from the same localStorage key WhatsApp writes to, so we
    // always get the authoritative value regardless of DOM structure changes.
    m_webEngine->page()->runJavaScript(
        "(function(){"
        "  var v = localStorage.getItem('theme');"
        "  try { v = JSON.parse(v); } catch(e) {}"  // handle both 'dark' and '"dark"'
        "  return v === 'dark' ? 'dark' : 'light';"
        "})();",
        [=](const QVariant &result) {
          theme = result.toString() == "dark" ? "dark" : "light";
          SettingsManager::instance().settings().setValue("windowTheme", theme);
        });
  }
  return theme;
}

// ── Fullscreen ────────────────────────────────────────────────────────────────

void MainWindow::fullScreenRequested(QWebEngineFullScreenRequest request) {
  if (request.toggleOn()) {
    windowStateBeforeFullScreen = this->windowState();
    this->hide();
    m_webEngine->showFullScreen();
    m_webEngine->setWindowState(Qt::WindowFullScreen);
    this->setWindowState(Qt::WindowFullScreen);
    this->show();
  } else {
    this->hide();
    m_webEngine->showNormal();
    this->setWindowState(windowStateBeforeFullScreen);
    this->show();
  }
  request.accept();
}

// ── Misc web engine helpers ───────────────────────────────────────────────────

void MainWindow::toggleMute(const bool &checked) {
  m_webEngine->page()->setAudioMuted(checked);
}
