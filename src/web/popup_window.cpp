#include "web/popup_window.h"

#include "core/navigation_policy.h"
#include "web/logging.h"
#include "web/web_profile.h"

#include <QDesktopServices>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QWebEngineFullScreenRequest>
#include <QWebEnginePage>
#include <QWebEngineView>

using namespace Qt::StringLiterals;

namespace whatsie::web {

/// First navigation decides the fate of the window.
class PopupWindow::Page : public QWebEnginePage
{
public:
    Page(WebProfile& profile, PopupWindow* window)
        : QWebEnginePage(&profile, window)
        , m_window(window)
    {}

protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override
    {
        if (isMainFrame && core::shouldOpenExternally(url)) {
            qCInfo(lcWeb) << "popup → system browser:" << url;
            QDesktopServices::openUrl(url);
            m_window->close();
            return false;
        }
        if (isMainFrame && !core::isWhatsAppWebUrl(url) && !url.isEmpty() && url.scheme() != u"about"_s &&
            url.scheme() != u"blob"_s) {
            qCDebug(lcWeb) << "popup navigation blocked:" << url;
            m_window->close();
            return false;
        }
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }

private:
    PopupWindow* m_window;
};

PopupWindow::PopupWindow(WebProfile& profile, QWidget* parent)
    : QWidget(parent, Qt::Window)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(u"WhatsApp"_s);
    resize(900, 640);

    m_view = new QWebEngineView(this);
    auto* page = new Page(profile, this);
    m_view->setPage(page);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);

    connect(page, &QWebEnginePage::titleChanged, this, &QWidget::setWindowTitle);
    connect(page, &QWebEnginePage::windowCloseRequested, this, &QWidget::close);
    connect(page, &QWebEnginePage::fullScreenRequested, this, [this](QWebEngineFullScreenRequest request) {
        request.accept();
        if (request.toggleOn()) {
            m_stateBeforeFullScreen = windowState();
            showFullScreen();
        } else {
            setWindowState(m_stateBeforeFullScreen & ~Qt::WindowFullScreen);
        }
    });
    qCInfo(lcWeb) << "popup window created";
}

QWebEnginePage* PopupWindow::page() const
{
    return m_view->page();
}

void PopupWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        if (isFullScreen()) {
            setWindowState(m_stateBeforeFullScreen & ~Qt::WindowFullScreen);
        } else {
            close();
        }
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

} // namespace whatsie::web
