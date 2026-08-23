#include "webenginepage.h"
#include "webengineprofilemanager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>

WebEnginePage::WebEnginePage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent) {

  auto userAgent = profile->httpUserAgent();
  qDebug() << "WebEnginePage::Profile::UserAgent" << userAgent;
  auto webengineversion =
      userAgent.split("QtWebEngine").last().split(" ").first();
  auto toRemove = "QtWebEngine" + webengineversion;
  auto cleanUserAgent = userAgent.remove(toRemove).replace("  ", " ");
  profile->setHttpUserAgent(cleanUserAgent);

  connect(this, &QWebEnginePage::loadFinished, this,
          &WebEnginePage::handleLoadFinished);
  connect(this, &QWebEnginePage::authenticationRequired, this,
          &WebEnginePage::handleAuthenticationRequired);
  connect(this, &QWebEnginePage::permissionRequested, this,
          &WebEnginePage::handlePermissionRequested);
  connect(this, &QWebEnginePage::proxyAuthenticationRequired, this,
          &WebEnginePage::handleProxyAuthenticationRequired);
  connect(this, &QWebEnginePage::registerProtocolHandlerRequested, this,
          &WebEnginePage::handleRegisterProtocolHandlerRequested);
  connect(this, &QWebEnginePage::selectClientCertificate, this,
          &WebEnginePage::handleSelectClientCertificate);
  connect(this, &QWebEnginePage::certificateError, this,
          &WebEnginePage::handleCertificateError);
}

bool WebEnginePage::acceptNavigationRequest(const QUrl &url,
                                            QWebEnginePage::NavigationType type,
                                            bool isMainFrame) {
  // Open link clicks in the default browser — but only when they leave
  // WhatsApp. The logout flow ends with a click-triggered navigation back to
  // web.whatsapp.com; hijacking it opened a browser tab and left the app
  // stuck on the "Logging out" overlay forever.
  if (QWebEnginePage::NavigationType::NavigationTypeLinkClicked == type &&
      url.host() != QLatin1String("web.whatsapp.com")) {
    QDesktopServices::openUrl(url);
    return false;
  }

  return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

QWebEnginePage *
WebEnginePage::createWindow(QWebEnginePage::WebWindowType type) {
  Q_UNUSED(type);
  return new WebEnginePage(this->profile());
}

inline QString questionForPermission(const QWebEnginePermission &permission) {
  switch (permission.permissionType()) {
  case QWebEnginePermission::PermissionType::Geolocation:
    return WebEnginePage::tr("Allow %1 to access your location information?");
  case QWebEnginePermission::PermissionType::MediaAudioCapture:
    return WebEnginePage::tr("Allow %1 to access your microphone?");
  case QWebEnginePermission::PermissionType::MediaVideoCapture:
    return WebEnginePage::tr("Allow %1 to access your webcam?");
  case QWebEnginePermission::PermissionType::MediaAudioVideoCapture:
    return WebEnginePage::tr("Allow %1 to access your microphone and webcam?");
  case QWebEnginePermission::PermissionType::MouseLock:
    return WebEnginePage::tr("Allow %1 to lock your mouse cursor?");
  case QWebEnginePermission::PermissionType::DesktopVideoCapture:
    return WebEnginePage::tr("Allow %1 to capture video of your desktop?");
  case QWebEnginePermission::PermissionType::DesktopAudioVideoCapture:
    return WebEnginePage::tr(
        "Allow %1 to capture audio and video of your desktop?");
  case QWebEnginePermission::PermissionType::Notifications:
    return WebEnginePage::tr("Allow %1 to show notification on your desktop?");
  default:
    return QString();
  }
}

void WebEnginePage::handlePermissionRequested(QWebEnginePermission permission) {
  bool autoPlay = true;
  if (SettingsManager::instance().settings().value("autoPlayMedia").isValid())
    autoPlay = SettingsManager::instance()
                   .settings()
                   .value("autoPlayMedia", false)
                   .toBool();

  if (autoPlay && (permission.permissionType() == QWebEnginePermission::PermissionType::MediaVideoCapture ||
                   permission.permissionType() == QWebEnginePermission::PermissionType::MediaAudioVideoCapture)) {
    WebEngineProfileManager::instance().profile()->settings()
        ->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
  }

  QString title = tr("Permission Request");
  QString question = questionForPermission(permission).arg(permission.origin().host());

  QString permissionTypeStr = QString::number(static_cast<int>(permission.permissionType()));
  SettingsManager::instance().settings().beginGroup("permissions");

  if (SettingsManager::instance().settings().value(permissionTypeStr, false).toBool()) {
    permission.grant();
  } else {
    if (!question.isEmpty() &&
        QMessageBox::question(view()->window(), title, question) == QMessageBox::Yes) {
      permission.grant();
      SettingsManager::instance().settings().setValue(permissionTypeStr, true);
    } else {
      permission.deny();
      SettingsManager::instance().settings().setValue(permissionTypeStr, false);
    }
  }
  SettingsManager::instance().settings().endGroup();
}

void WebEnginePage::handleLoadFinished(bool ok) {

  // turn on Notification settings by default
  if (SettingsManager::instance()
          .settings()
          .value("permissions/Notifications")
          .isValid() == false) {
    SettingsManager::instance().settings().beginGroup("permissions");
    SettingsManager::instance().settings().setValue("Notifications", true);
    SettingsManager::instance().settings().endGroup();
  }

  if (ok) {
    injectPreventScrollWheelZoomHelper();
    injectNewChatJavaScript();
    injectReplyPreviewUpdater();
  }
}

void WebEnginePage::fullScreenRequestedByPage(
    QWebEngineFullScreenRequest request) {
  request.accept();
}

QStringList WebEnginePage::chooseFiles(QWebEnginePage::FileSelectionMode mode,
                                       const QStringList &oldFiles,
                                       const QStringList &acceptedMimeTypes) {
  qDebug() << mode << oldFiles << acceptedMimeTypes;
  QFileDialog::FileMode dialogMode;
  if (mode == QWebEnginePage::FileSelectOpen) {
    dialogMode = QFileDialog::ExistingFile;
  } else {
    dialogMode = QFileDialog::ExistingFiles;
  }

  QFileDialog *dialog = new QFileDialog();
  bool usenativeFileDialog = SettingsManager::instance()
                                 .settings()
                                 .value("useNativeFileDialog", false)
                                 .toBool();

  if (usenativeFileDialog == false) {
    dialog->setOption(QFileDialog::DontUseNativeDialog, true);
  }
  dialog->setFileMode(dialogMode);
  QStringList mimeFilters;
  mimeFilters.append("application/octet-stream"); // to show All files(*)
  mimeFilters.append(acceptedMimeTypes);

  if (acceptedMimeTypes.contains("image/*")) {
    foreach (QByteArray mime, QImageReader::supportedImageFormats()) {
      mimeFilters.append("image/" + mime);
    }
  }

  mimeFilters.sort(Qt::CaseSensitive);
  dialog->setMimeTypeFilters(mimeFilters);

  QStringList selectedFiles;
  if (dialog->exec()) {
    selectedFiles = dialog->selectedFiles();
  }
  dialog->deleteLater();
  return selectedFiles;
}

void WebEnginePage::handleCertificateError(
    const QWebEngineCertificateError &error) {
  QString description = error.description();
  QWidget *mainWindow = view()->window();
  if (error.isOverridable()) {
    QDialog dialog(mainWindow);
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() &
                          ~Qt::WindowContextHelpButtonHint);
    Ui::CertificateErrorDialog certificateDialog;
    certificateDialog.setupUi(&dialog);
    certificateDialog.m_iconLabel->setText(QString());
    QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxWarning,
                                                 nullptr, mainWindow));
    certificateDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));
    certificateDialog.m_errorLabel->setText(description);
    dialog.setWindowTitle(tr("Certificate Error"));
    bool accepted = dialog.exec() == QDialog::Accepted;
    auto handler = const_cast<QWebEngineCertificateError &>(error);
    if (accepted)
      handler.acceptCertificate();
    else
      handler.rejectCertificate();
  }

  QMessageBox::critical(mainWindow, tr("Certificate Error"), description);
}

void WebEnginePage::handleAuthenticationRequired(const QUrl &requestUrl,
                                                 QAuthenticator *auth) {
  QWidget *mainWindow = view()->window();
  QDialog dialog(mainWindow);
  dialog.setModal(true);
  dialog.setWindowFlags(dialog.windowFlags() &
                        ~Qt::WindowContextHelpButtonHint);

  Ui::PasswordDialog passwordDialog;
  passwordDialog.setupUi(&dialog);

  passwordDialog.m_iconLabel->setText(QString());
  QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion,
                                               nullptr, mainWindow));
  passwordDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));

  QString introMessage(
      tr("Enter username and password for \"%1\" at %2")
          .arg(auth->realm(), requestUrl.toString().toHtmlEscaped()));
  passwordDialog.m_infoLabel->setText(introMessage);
  passwordDialog.m_infoLabel->setWordWrap(true);

  if (dialog.exec() == QDialog::Accepted) {
    auth->setUser(passwordDialog.m_userNameLineEdit->text());
    auth->setPassword(passwordDialog.m_passwordLineEdit->text());
  } else {
    // Set authenticator null if dialog is cancelled
    *auth = QAuthenticator();
  }
}

void WebEnginePage::handleProxyAuthenticationRequired(
    const QUrl &, QAuthenticator *auth, const QString &proxyHost) {
  QWidget *mainWindow = view()->window();
  QDialog dialog(mainWindow);
  dialog.setModal(true);
  dialog.setWindowFlags(dialog.windowFlags() &
                        ~Qt::WindowContextHelpButtonHint);

  Ui::PasswordDialog passwordDialog;
  passwordDialog.setupUi(&dialog);

  passwordDialog.m_iconLabel->setText(QString());
  QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion,
                                               nullptr, mainWindow));
  passwordDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));

  QString introMessage = tr("Connect to proxy \"%1\" using:");
  introMessage = introMessage.arg(proxyHost.toHtmlEscaped());
  passwordDialog.m_infoLabel->setText(introMessage);
  passwordDialog.m_infoLabel->setWordWrap(true);

  if (dialog.exec() == QDialog::Accepted) {
    auth->setUser(passwordDialog.m_userNameLineEdit->text());
    auth->setPassword(passwordDialog.m_passwordLineEdit->text());
  } else {
    // Set authenticator null if dialog is cancelled
    *auth = QAuthenticator();
  }
}

//! [registerProtocolHandlerRequested]
void WebEnginePage::handleRegisterProtocolHandlerRequested(
    QWebEngineRegisterProtocolHandlerRequest request) {
  auto answer = QMessageBox::question(
      view()->window(), tr("Permission Request"),
      tr("Allow %1 to open all %2 links?")
          .arg(request.origin().host(), request.scheme()));
  if (answer == QMessageBox::Yes)
    request.accept();
  else
    request.reject();
}
//! [registerProtocolHandlerRequested]

void WebEnginePage::handleSelectClientCertificate(
    QWebEngineClientCertificateSelection selection) {
  // Just select one.
  selection.select(selection.certificates().at(0));

  qDebug() << __FUNCTION__;
  auto certificates = selection.certificates();
  for (const QSslCertificate &cert : std::as_const(certificates)) {
    qDebug() << cert;
    selection.select(cert); // select the first available cert
    break;
  }
  qDebug() << selection.host();
}

void WebEnginePage::javaScriptConsoleMessage(
    WebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message,
    int lineId, const QString &sourceId) {
  Q_UNUSED(level);
  Q_UNUSED(message);
  Q_UNUSED(lineId);
  Q_UNUSED(sourceId);
}

void WebEnginePage::injectPreventScrollWheelZoomHelper() {
  QString js = R"(
                    (function () {
                        const SSWZ = function () {
                            this.keyScrollHandler = function (e) {
                                if (e.ctrlKey) {
                                    e.preventDefault();
                                    return false;
                                }
                            }
                        };
                        if (window === top) {
                            const sswz = new SSWZ();
                            window.addEventListener('wheel', sswz.keyScrollHandler, {
                                passive: false
                            });
                        }
                    })();
                )";
  this->runJavaScript(js);
}

QString WebEnginePage::loadReplyUpdaterConfig() {
  // Try external config first (allows updates without rebuild)
  QStringList searchPaths = {
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation),
      QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
          QStringLiteral("/whatsie"),
      QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
          QStringLiteral("/.config/whatsie")};

  for (const QString &path : searchPaths) {
    QFile file(path + QStringLiteral("/reply-updater-config.json"));
    if (file.open(QIODevice::ReadOnly)) {
      QByteArray data = file.readAll();
      file.close();
      qDebug() << "Reply updater config loaded from:" << file.fileName();
      return QString::fromUtf8(data);
    }
  }

  // Fallback: use bundled resource
  QFile bundled(QStringLiteral(":/reply-updater-config.json"));
  if (bundled.open(QIODevice::ReadOnly)) {
    QByteArray data = bundled.readAll();
    bundled.close();
    return QString::fromUtf8(data);
  }

  // Ultimate fallback: hardcoded defaults
  return QStringLiteral(R"json({
    "msgStoreAccess": [
      "require('WAWebCollections').Msg",
      "require('WAWebStore').Msg",
      "require('WAWebMsgStore')",
      "window.Store && window.Store.Msg"
    ],
    "quotedMessageSelectors": [
      "[data-testid='quoted-message']",
      "div[class*='quoted-mention']",
      "div[aria-label*='Quoted message']",
      "div._aju3"
    ],
    "quotedMessageTextSelectors": [
      "[data-testid='quoted-message'] span.selectable-text",
      "div._aju3 span.selectable-text",
      ".quoted-msg-text",
      "span._ao3q"
    ],
    "messageContainerSelector": [
      "[data-testid='msg-list']",
      "#main"
    ],
    "msgIdAttributes": ["data-id", "data-ref-id", "data-original-msg-id"],
    "quoteRefAttributes": ["data-id", "data-ref-id", "data-original-msg-id", "data-msg-id"],
    "quoteRefLinkSelectors": ["a[href*='message/']", "[role='button'][data-id]"]
  })json");
}

void WebEnginePage::injectReplyPreviewUpdater() {
  QString config = loadReplyUpdaterConfig();
  // Escape for embedding in JS string literal
  config.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
  config.replace(QLatin1Char('\''), QStringLiteral("\\'"));
  config.replace(QLatin1Char('\n'), QStringLiteral("\\n"));
  config.replace(QLatin1Char('\r'), QStringLiteral("\\r"));

  const QString js = QStringLiteral(R"js(
    (function(cfg) {
      'use strict';
      try {
        // ── Parse config ──────────────────────────────────────────────
        var C;
        try { C = JSON.parse(cfg); } catch(e) { return; }

        // ── Layer 1: Access message store (configurable) ──────────────
        var msgStore = null;
        var storeExprs = C.msgStoreAccess || [];
        for (var i = 0; i < storeExprs.length; i++) {
          try { msgStore = eval(storeExprs[i]); } catch(e) {}
          if (msgStore) break;
        }
        if (!msgStore) return; // WhatsApp structure changed — no-op

        // ── Layer 2: Helper — try multiple selectors, return first match ─
        function trySelectors(parent, selectors) {
          if (!parent || !selectors) return null;
          for (var i = 0; i < selectors.length; i++) {
            try {
              var el = parent.querySelector(selectors[i]);
              if (el) return el;
            } catch(e) {}
          }
          return null;
        }

        function trySelectorsAll(parent, selectors) {
          if (!parent || !selectors) return [];
          for (var i = 0; i < selectors.length; i++) {
            try {
              var els = parent.querySelectorAll(selectors[i]);
              if (els.length > 0) return Array.from(els);
            } catch(e) {}
          }
          return [];
        }

        // ── Layer 3: Extract message text from node ───────────────────
        function getMsgText(node) {
          if (!node) return null;
          // Try configured selectors first
          var el = trySelectors(node, C.quotedMessageTextSelectors);
          if (el) return el.textContent;
          // Fallback: walk spans
          try {
            var spans = node.querySelectorAll('span');
            for (var i = spans.length - 1; i >= 0; i--) {
              var t = spans[i].textContent.trim();
              if (t.length > 0 && !spans[i].querySelector('span')) return t;
            }
          } catch(e) {}
          return null;
        }

        // ── Layer 4: Get message ID from a reply quote container ──────
        function getQuoteRefMsgId(quoteContainer) {
          if (!quoteContainer) return null;
          try {
            // Try configured attributes
            var attrs = C.quoteRefAttributes || ['data-id'];
            for (var i = 0; i < attrs.length; i++) {
              var val = quoteContainer.getAttribute(attrs[i]);
              if (val) return val;
            }
            // Try configured link selectors
            var links = C.quoteRefLinkSelectors || ['a[href*="message/"]'];
            for (var i = 0; i < links.length; i++) {
              var link = quoteContainer.querySelector(links[i]);
              if (link) {
                return link.getAttribute('data-id')
                    || (link.href && link.href.split('/').pop());
              }
            }
          } catch(e) {}
          return null;
        }

        // ── Layer 5: Get message body from store object ───────────────
        function getMsgBody(msg) {
          if (!msg) return null;
          // WhatsApp message objects have different structures across versions
          var props = ['body', 'text', 'content', 'message'];
          for (var i = 0; i < props.length; i++) {
            if (msg[props[i]] && typeof msg[props[i]] === 'string')
              return msg[props[i]];
          }
          // Try __x wrapper (older versions)
          if (msg.__x) {
            for (var i = 0; i < props.length; i++) {
              if (msg.__x[props[i]] && typeof msg.__x[props[i]] === 'string')
                return msg.__x[props[i]];
            }
          }
          return null;
        }

        // ── Layer 6: Check if message is edited ───────────────────────
        function isEdited(msg) {
          if (!msg) return false;
          if (msg.edited === true || msg.isEdited === true) return true;
          if (msg.__x && (msg.__x.edited === true || msg.__x.isEdited === true))
            return true;
          // Check for edited timestamp
          if (msg.editedTimestamp || msg.t !== undefined) {
            // Some versions use tsChanged > t to indicate edit
            if (msg.tsChanged && msg.t && msg.tsChanged > msg.t) return true;
          }
          return false;
        }

        // ── Layer 7: Update all reply quotes for a given message ──────
        function updateReplyPreviews(msgId, newText) {
          if (!msgId || !newText) return;
          var quotes = trySelectorsAll(document, C.quotedMessageSelectors);
          for (var i = 0; i < quotes.length; i++) {
            var refId = getQuoteRefMsgId(quotes[i]);
            if (refId && refId === msgId) {
              var textEl = trySelectors(quotes[i], C.quotedMessageTextSelectors);
              if (textEl && textEl.textContent !== newText) {
                textEl.textContent = newText;
                console.log('[WhatSie] Updated reply preview for:', msgId);
              }
            }
          }
        }

        // ── Layer 8: MutationObserver — watch for DOM changes ─────────
        var targetNode = trySelectors(document, C.messageContainerSelector);
        if (!targetNode) return; // WhatsApp structure changed — no-op

        var lastScan = 0;
        var DEBOUNCE_MS = 1000;

        var observer = new MutationObserver(function(mutations) {
          try {
            var now = Date.now();
            if (now - lastScan < DEBOUNCE_MS) return;
            lastScan = now;

            for (var m = 0; m < mutations.length; m++) {
              var addedNodes = mutations[m].addedNodes;
              for (var n = 0; n < addedNodes.length; n++) {
                var node = addedNodes[n];
                if (node.nodeType !== 1) continue;

                // Scan all reply quotes in added nodes
                var quotes = trySelectorsAll(node, C.quotedMessageSelectors);
                for (var q = 0; q < quotes.length; q++) {
                  var refId = getQuoteRefMsgId(quotes[q]);
                  if (!refId) continue;
                  try {
                    var origMsg = msgStore.get(refId);
                    if (origMsg) {
                      var body = getMsgBody(origMsg);
                      if (body) {
                        var textEl = trySelectors(quotes[q], C.quotedMessageTextSelectors);
                        if (textEl && textEl.textContent !== body) {
                          textEl.textContent = body;
                          console.log('[WhatSie] Updated quote preview:', refId);
                        }
                      }
                    }
                  } catch(e) {}
                }

                // Also scan the node itself if it's a message container
                var msgId = node.getAttribute && node.getAttribute('data-id');
                if (msgId) {
                  try {
                    var msg = msgStore.get(msgId);
                    if (msg && isEdited(msg)) {
                      var body = getMsgBody(msg);
                      if (body) updateReplyPreviews(msgId, body);
                    }
                  } catch(e) {}
                }
              }
            }
          } catch(e) { /* observer continues */ }
        });

        observer.observe(targetNode, {
          childList: true,
          subtree: true,
          attributes: false
        });

        // Auto-disconnect after 5 minutes (page reload re-injects)
        setTimeout(function() {
          try { observer.disconnect(); } catch(e) {}
        }, 300000);

        console.log('[WhatSie] Reply preview updater active');

      } catch(e) {
        // Total failure — silent no-op. App is unaffected.
      }
    })('%1');
  )js").arg(config);

  this->runJavaScript(js);
}

void WebEnginePage::injectNewChatJavaScript() {
  QString js = R"(const openNewChatWhatsie = (phone,text) => {
                    const link = document.createElement('a');
                    link.setAttribute('href',
                    `whatsapp://send/?phone=${phone}&text=${text}`);
                    document.body.append(link);
                    link.click();
                    document.body.removeChild(link);
                };
                function openNewChatWhatsieDefined()
                {
                    return (openNewChatWhatsie != 'undefined');
                })";
  this->runJavaScript(js);
}
