#ifndef WEBENGINEPAGE_H
#define WEBENGINEPAGE_H

#include <QAuthenticator>
#include <QDesktopServices>
#include <QFileDialog>
#include <QIcon>
#include <QImageReader>
#include <QMessageBox>
#include <QStyle>
#include <QWebEngineCertificateError>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineNotification>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineRegisterProtocolHandlerRequest>
#include <QWebEngineSettings>

#include "settingsmanager.h"

#include "ui_certificateerrordialog.h"
#include "ui_passworddialog.h"

class WebEnginePage : public QWebEnginePage {
  Q_OBJECT
public:
  WebEnginePage(QWebEngineProfile *profile, QObject *parent = nullptr);
  void injectClassChangeObserver();

protected:
  bool acceptNavigationRequest(const QUrl &url,
                               QWebEnginePage::NavigationType type,
                               bool isMainFrame) override;
  QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) override;
  bool certificateError(const QWebEngineCertificateError &error) override;
  QStringList chooseFiles(FileSelectionMode mode, const QStringList &oldFiles,
                          const QStringList &acceptedMimeTypes) override;

public slots:
  void handleFeaturePermissionRequested(const QUrl &securityOrigin,
                                        QWebEnginePage::Feature feature);
  void handleLoadFinished(bool ok);

protected slots:
  void
  javaScriptConsoleMessage(WebEnginePage::JavaScriptConsoleMessageLevel level,
                           const QString &message, int lineId,
                           const QString &sourceId) override;
private slots:
  void handleAuthenticationRequired(const QUrl &requestUrl,
                                    QAuthenticator *auth);
  void handleProxyAuthenticationRequired(const QUrl &requestUrl,
                                         QAuthenticator *auth,
                                         const QString &proxyHost);
  void handleRegisterProtocolHandlerRequested(
      QWebEngineRegisterProtocolHandlerRequest request);
#if !defined(QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
  void handleSelectClientCertificate(
      QWebEngineClientCertificateSelection clientCertSelection);
#endif
  void fullScreenRequestedByPage(QWebEngineFullScreenRequest request);
  void injectPreventScrollWheelZoomHelper();
  void injectFullWidthJavaScript();
  void injectNewChatJavaScript();
};

#endif // WEBENGINEPAGE_H
