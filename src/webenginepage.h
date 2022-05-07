#ifndef WEBENGINEPAGE_H
#define WEBENGINEPAGE_H

#include <QFileDialog>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineNotification>
#include <QWebEngineProfile>

#include <QAuthenticator>
#include <QDesktopServices>
#include <QImageReader>
#include <QMessageBox>
#include <QWebEngineCertificateError>
#include <QWebEnginePage>

#include <QWebEngineFullScreenRequest>
#include <QWebEngineRegisterProtocolHandlerRequest>

#include <QSettings>

#include "ui_certificateerrordialog.h"
#include "ui_passworddialog.h"

class WebEnginePage : public QWebEnginePage {
  Q_OBJECT
public:
  WebEnginePage(QWebEngineProfile *profile, QObject *parent = nullptr);

private:
  QSettings settings;

protected:
  bool acceptNavigationRequest(const QUrl &url,
                               QWebEnginePage::NavigationType type,
                               bool isMainFrame) override;
  QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) override;
  bool certificateError(const QWebEngineCertificateError &error) override;
  QStringList chooseFiles(FileSelectionMode mode, const QStringList &oldFiles,
                          const QStringList &acceptedMimeTypes);

public slots:
  void handleFeaturePermissionRequested(const QUrl &securityOrigin,
                                        QWebEnginePage::Feature feature);
  void handleLoadFinished(bool ok);

protected slots:
  void
  javaScriptConsoleMessage(WebEnginePage::JavaScriptConsoleMessageLevel level,
                           const QString &message, int lineId,
                           const QString &sourceId);
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
};

#endif // WEBENGINEPAGE_H
