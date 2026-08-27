#include "web/web_page.h"

#include "web/logging.h"
#include "web/web_profile.h"

namespace whatsie::web {

WebPage::WebPage(WebProfile& profile, QObject* parent)
    : QWebEnginePage(&profile, parent)
{}

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
