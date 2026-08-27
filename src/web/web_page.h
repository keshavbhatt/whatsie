#pragma once

#include <QWebEnginePage>

namespace whatsie::web {

class WebProfile;

/// The page hosting WhatsApp Web. Routes the page console to the
/// `whatsie.web.js` logging category, sends link clicks that leave
/// web.whatsapp.com to the system browser (FEATURES M4) and handles
/// window.open() targets the same way.
class WebPage : public QWebEnginePage
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WebPage)

public:
    explicit WebPage(WebProfile& profile, QObject* parent = nullptr);
    ~WebPage() override = default;

Q_SIGNALS:
    /// A popup pointing back at web.whatsapp.com (e.g. a call window). Kept
    /// in-app from M3 on; until then the UI decides what to do.
    void inAppPopupRequested(const QUrl& url);

protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;
    QWebEnginePage* createWindow(WebWindowType type) override;
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber,
                                  const QString& sourceId) override;
};

} // namespace whatsie::web
