#ifndef WEBENGINEPAGE_H
#define WEBENGINEPAGE_H


#include <QFileDialog>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineNotification>
#include <QWebEngineProfile>

#include <QWebEnginePage>
#include <QDesktopServices>
#include <QMessageBox>
#include <QImageReader>
#include <QWebEngineCertificateError>
#include <QAuthenticator>

#include <QWebEngineRegisterProtocolHandlerRequest>
#include <QWebEngineFullScreenRequest>

#include "ui_certificateerrordialog.h"
#include "ui_passworddialog.h"

class WebEnginePage : public QWebEnginePage
{
    Q_OBJECT
public:
    WebEnginePage(QWebEngineProfile *profile, QObject *parent = nullptr);

protected:
    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override;
    QWebEnginePage* createWindow(QWebEnginePage::WebWindowType type) override;
    bool certificateError(const QWebEngineCertificateError &error) override;
    QStringList chooseFiles(FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes);

public slots:
    void handleFeaturePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature);
    void handleLoadFinished(bool ok);

private slots:
    void handleAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth);
    void handleProxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost);
    void handleRegisterProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest request);
#if !defined(QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    void handleSelectClientCertificate(QWebEngineClientCertificateSelection clientCertSelection);
#endif
    void fullScreenRequestedByPage(QWebEngineFullScreenRequest request);
};

#endif // WEBENGINEPAGE_H
