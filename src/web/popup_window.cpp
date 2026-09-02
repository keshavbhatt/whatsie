#include "web/popup_window.h"

#include "core/navigation_policy.h"
#include "platform/file_manager.h"
#include "web/logging.h"
#include "web/web_profile.h"

#include <QKeyEvent>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineScript>
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
        // The Adobe Acrobat PDF integration must load in-app: WhatsApp transfers
        // the PDF to this window over postMessage, which breaks the moment the
        // window is closed or handed to the browser. Let it (and its sign-in
        // flow) through both guards below.
        if (isMainFrame && core::isPdfIntegrationUrl(url)) {
            return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
        }
        if (isMainFrame && core::shouldOpenExternally(url)) {
            qCInfo(lcWeb) << "popup → system browser:" << url;
            platform::openUrl(url.toString());
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
            m_exitFullScreen->setEnabled(true);
        } else {
            exitFullScreen();
        }
    });
    // Esc leaves full screen even while the web view holds keyboard focus (a
    // plain keyPressEvent never reaches us then); only active in full screen so
    // it does not shadow Esc otherwise. FEATURES M3.
    m_exitFullScreen = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    m_exitFullScreen->setContext(Qt::WindowShortcut);
    m_exitFullScreen->setEnabled(false);
    connect(m_exitFullScreen, &QShortcut::activated, this, &PopupWindow::exitFullScreen);
    qCInfo(lcWeb) << "popup window created";
}

QWebEnginePage* PopupWindow::page() const
{
    return m_view->page();
}

void PopupWindow::exitFullScreen()
{
    m_exitFullScreen->setEnabled(false);
    // Explicit restore (clearing the flag via setWindowState is unreliable on
    // Wayland) and sync the page out of HTML full screen.
    if (m_stateBeforeFullScreen & Qt::WindowMaximized) {
        showMaximized();
    } else {
        showNormal();
    }
    page()->runJavaScript(u"if (document.fullscreenElement) { document.exitFullscreen(); }"_s,
                          QWebEngineScript::MainWorld);
}

void PopupWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        if (isFullScreen()) {
            exitFullScreen();
        } else {
            close();
        }
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

} // namespace whatsie::web
