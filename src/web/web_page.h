#pragma once

#include <QWebEnginePage>

namespace whatsie::web {

class WebProfile;

/// The page hosting WhatsApp Web. Routes the page console to the
/// `whatsie.web.js` logging category so breakage in WhatsApp Web or in our
/// injected scripts is never silent.
class WebPage : public QWebEnginePage
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WebPage)

public:
    explicit WebPage(WebProfile& profile, QObject* parent = nullptr);
    ~WebPage() override = default;

protected:
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber,
                                  const QString& sourceId) override;
};

} // namespace whatsie::web
