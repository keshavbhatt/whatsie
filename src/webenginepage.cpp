#include "webenginepage.h"

#include <QIcon>
#include <QStyle>


WebEnginePage::WebEnginePage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
{
    // Connect signals and slots
    connect(this, &QWebEnginePage::featurePermissionRequested,
            this, &WebEnginePage::handleFeaturePermissionRequested);
    connect(this, &QWebEnginePage::loadFinished,
            this, &WebEnginePage::handleLoadFinished);
    profile->setHttpUserAgent(profile->httpUserAgent().replace("QtWebEngine/5.13.0",""));
    connect(this, &QWebEnginePage::authenticationRequired, this, &WebEnginePage::handleAuthenticationRequired);
    connect(this, &QWebEnginePage::featurePermissionRequested, this, &WebEnginePage::handleFeaturePermissionRequested);
    connect(this, &QWebEnginePage::proxyAuthenticationRequired, this, &WebEnginePage::handleProxyAuthenticationRequired);
    connect(this, &QWebEnginePage::registerProtocolHandlerRequested, this, &WebEnginePage::handleRegisterProtocolHandlerRequested);

#if !defined(QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    connect(this, &QWebEnginePage::selectClientCertificate, this, &WebEnginePage::handleSelectClientCertificate);
#endif
}

bool WebEnginePage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    qDebug() << "Navigation request: [" + url.toDisplayString() + "] " + type;

    if (QWebEnginePage::NavigationType::NavigationTypeLinkClicked == type)
    {
        QDesktopServices::openUrl(url);
        return false;
    }

    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

QWebEnginePage *WebEnginePage::createWindow(QWebEnginePage::WebWindowType type)
{
    Q_UNUSED(type);
    return new WebEnginePage(this->profile());
}


inline QString questionForFeature(QWebEnginePage::Feature feature)
{
    switch (feature) {
    case QWebEnginePage::Geolocation:
        return WebEnginePage::tr("Allow %1 to access your location information?");
    case QWebEnginePage::MediaAudioCapture:
        return WebEnginePage::tr("Allow %1 to access your microphone?");
    case QWebEnginePage::MediaVideoCapture:
        return WebEnginePage::tr("Allow %1 to access your webcam?");
    case QWebEnginePage::MediaAudioVideoCapture:
        return WebEnginePage::tr("Allow %1 to access your microphone and webcam?");
    case QWebEnginePage::MouseLock:
        return WebEnginePage::tr("Allow %1 to lock your mouse cursor?");
    case QWebEnginePage::DesktopVideoCapture:
        return WebEnginePage::tr("Allow %1 to capture video of your desktop?");
    case QWebEnginePage::DesktopAudioVideoCapture:
        return WebEnginePage::tr("Allow %1 to capture audio and video of your desktop?");
    case QWebEnginePage::Notifications:
        return WebEnginePage::tr("Allow %1 to show notification on your desktop?");
    }
    return QString();
}

void WebEnginePage::handleFeaturePermissionRequested(const QUrl &securityOrigin, Feature feature)
{
    QString title = tr("Permission Request");
    QString question = questionForFeature(feature).arg(securityOrigin.host());
    if (!question.isEmpty() && QMessageBox::question(view()->window(), title, question) == QMessageBox::Yes)
        setFeaturePermission(securityOrigin, feature, PermissionGrantedByUser);
    else
        setFeaturePermission(securityOrigin, feature, PermissionDeniedByUser);
}

void WebEnginePage::handleLoadFinished(bool ok)
{
    Q_UNUSED(ok);
    setFeaturePermission(
                QUrl("https://web.whatsapp.com/"),
                QWebEnginePage::Feature::Notifications,
                QWebEnginePage::PermissionPolicy::PermissionGrantedByUser
    );
    setFeaturePermission(
                QUrl("https://web.whatsapp.com/"),
                QWebEnginePage::Feature::MediaAudioCapture,
                QWebEnginePage::PermissionPolicy::PermissionGrantedByUser
    );
    setFeaturePermission(
                QUrl("https://web.whatsapp.com/"),
                QWebEnginePage::Feature::MediaVideoCapture,
                QWebEnginePage::PermissionPolicy::PermissionGrantedByUser
    );
    setFeaturePermission(
                QUrl("https://web.whatsapp.com/"),
                QWebEnginePage::Feature::MediaAudioVideoCapture,
                QWebEnginePage::PermissionPolicy::PermissionGrantedByUser
    );
}

void WebEnginePage::fullScreenRequestedByPage(QWebEngineFullScreenRequest request)
{
    qDebug()<<"Fullscreen";
    request.accept();
}

QStringList WebEnginePage::chooseFiles(QWebEnginePage::FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes)
{
    qDebug()<<mode<<oldFiles<<acceptedMimeTypes;
    QFileDialog::FileMode dialogMode;
        if(mode == QWebEnginePage::FileSelectOpen) {
            dialogMode = QFileDialog::ExistingFile;
        } else {
            dialogMode = QFileDialog::ExistingFiles;
        }

        QFileDialog* dialog = new QFileDialog();

        dialog->setFileMode(dialogMode);
        dialog->setOption(QFileDialog::DontUseNativeDialog,true);

        QStringList mimeFilters = acceptedMimeTypes;

        if(acceptedMimeTypes.contains("image/*")){
            foreach(QByteArray mime,QImageReader::supportedImageFormats()){
                    mimeFilters.append("image/"+mime);
            }
        }

        dialog->setMimeTypeFilters(mimeFilters);

        QStringList selectedFiles;
        if(dialog->exec()) {
            selectedFiles = dialog->selectedFiles();
        }

        return selectedFiles;
}

bool WebEnginePage::certificateError(const QWebEngineCertificateError &error)
{
    QWidget *mainWindow = view()->window();
    if (error.isOverridable()) {
        QDialog dialog(mainWindow);
        dialog.setModal(true);
        dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        Ui::CertificateErrorDialog certificateDialog;
        certificateDialog.setupUi(&dialog);
        certificateDialog.m_iconLabel->setText(QString());
        QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxWarning, nullptr, mainWindow));
        certificateDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));
        certificateDialog.m_errorLabel->setText(error.errorDescription());
        dialog.setWindowTitle(tr("Certificate Error"));
        return dialog.exec() == QDialog::Accepted;
    }

    QMessageBox::critical(mainWindow, tr("Certificate Error"), error.errorDescription());
    return false;
}

void WebEnginePage::handleAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth)
{
    QWidget *mainWindow = view()->window();
    QDialog dialog(mainWindow);
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.m_iconLabel->setText(QString());
    QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, nullptr, mainWindow));
    passwordDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));

    QString introMessage(tr("Enter username and password for \"%1\" at %2")
                         .arg(auth->realm()).arg(requestUrl.toString().toHtmlEscaped()));
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

void WebEnginePage::handleProxyAuthenticationRequired(const QUrl &, QAuthenticator *auth, const QString &proxyHost)
{
    QWidget *mainWindow = view()->window();
    QDialog dialog(mainWindow);
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.m_iconLabel->setText(QString());
    QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, nullptr, mainWindow));
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
void WebEnginePage::handleRegisterProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest request)
{
    auto answer = QMessageBox::question(
        view()->window(),
        tr("Permission Request"),
        tr("Allow %1 to open all %2 links?")
        .arg(request.origin().host())
        .arg(request.scheme()));
    if (answer == QMessageBox::Yes)
        request.accept();
    else
        request.reject();
}
//! [registerProtocolHandlerRequested]

#if !defined(QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
void WebEnginePage::handleSelectClientCertificate(QWebEngineClientCertificateSelection selection)
{
    // Just select one.
    selection.select(selection.certificates().at(0));

    qDebug() << __FUNCTION__;
        for(QSslCertificate cert : selection.certificates()) {
            qDebug() << cert;
            selection.select(cert); // select the first available cert
            break;
        }
        qDebug() << selection.host();
}
#endif


