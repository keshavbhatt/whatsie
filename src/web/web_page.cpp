#include "web/web_page.h"

#include "core/navigation_policy.h"
#include "web/logging.h"
#include "web/web_profile.h"

#include <QDesktopServices>
#include <QWebEnginePermission>

namespace whatsie::web {

namespace {

/// Temporary page handed to Chromium for window.open()/target=_blank. It
/// never renders: the first navigation is inspected and either sent to the
/// system browser or reported back, then the page deletes itself.
class PopupPage : public QWebEnginePage
{
public:
    PopupPage(QWebEngineProfile* profile, WebPage* owner)
        : QWebEnginePage(profile, owner)
        , m_owner(owner)
    {}

protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType, bool) override
    {
        if (core::shouldOpenExternally(url)) {
            qCInfo(lcWeb) << "popup → system browser:" << url;
            QDesktopServices::openUrl(url);
        } else if (core::isWhatsAppWebUrl(url)) {
            qCInfo(lcWeb) << "popup stays in app:" << url;
            Q_EMIT m_owner->inAppPopupRequested(url);
        } else {
            qCDebug(lcWeb) << "popup navigation ignored:" << url;
        }
        deleteLater();
        return false;
    }

private:
    WebPage* m_owner;
};

} // namespace

WebPage::WebPage(WebProfile& profile, QObject* parent)
    : QWebEnginePage(&profile, parent)
{
    connect(this, &QWebEnginePage::permissionRequested, this, [](QWebEnginePermission permission) {
        // FEATURES N11: notifications are the point of the app — always allow.
        // Other permission types are decided by the M3 permission controller.
        if (permission.permissionType() == QWebEnginePermission::PermissionType::Notifications) {
            qCInfo(lcWeb) << "granting notification permission for" << permission.origin();
            permission.grant();
        }
    });
}

bool WebPage::acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame)
{
    if (isMainFrame && type == NavigationTypeLinkClicked && core::shouldOpenExternally(url)) {
        qCInfo(lcWeb) << "link → system browser:" << url;
        QDesktopServices::openUrl(url);
        return false;
    }
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

QWebEnginePage* WebPage::createWindow(WebWindowType type)
{
    Q_UNUSED(type)
    return new PopupPage(profile(), this);
}

void WebPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message,
                                       int lineNumber, const QString& sourceId)
{
    switch (level) {
    case InfoMessageLevel:
        qCDebug(lcWebJs).noquote() << sourceId << lineNumber << message;
        break;
    case WarningMessageLevel:
        qCWarning(lcWebJs).noquote() << sourceId << lineNumber << message;
        break;
    case ErrorMessageLevel:
        qCCritical(lcWebJs).noquote() << sourceId << lineNumber << message;
        break;
    }
}

} // namespace whatsie::web
