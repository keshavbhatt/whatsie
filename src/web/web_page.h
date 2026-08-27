#pragma once

#include <QWebEnginePage>

namespace whatsie::web {

class WebProfile;

/// The page hosting WhatsApp Web. Routes the page console to the
/// `whatsie.web.js` logging category, sends link clicks that leave
/// web.whatsapp.com to the system browser (FEATURES M4), hosts window.open()
/// targets in PopupWindows (M3) and provides the native file chooser (M5).
class WebPage : public QWebEnginePage
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WebPage)

public:
    explicit WebPage(WebProfile& profile, QObject* parent = nullptr);
    ~WebPage() override = default;

    /// Parent for pop-up windows and dialogs (the main window's view).
    void setHostWidget(QWidget* host) { m_host = host; }

Q_SIGNALS:
    void popupOpened(QWidget* window);

protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;
    QWebEnginePage* createWindow(WebWindowType type) override;
    QStringList chooseFiles(FileSelectionMode mode, const QStringList& oldFiles,
                            const QStringList& acceptedMimeTypes) override;
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber,
                                  const QString& sourceId) override;

private:
    WebProfile& m_profile;
    QWidget* m_host = nullptr;
};

/// Builds a QFileDialog name filter from the page's accepted MIME types /
/// extensions (pure, tested).
[[nodiscard]] QString nameFilterFor(const QStringList& acceptedMimeTypes);

} // namespace whatsie::web
