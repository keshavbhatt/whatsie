#include "web/popup_window.h"

#include "core/navigation_policy.h"
#include "platform/file_manager.h"
#include "web/logging.h"
#include "web/web_profile.h"

#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QLabel>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QShortcut>
#include <QTimer>
#include <QVBoxLayout>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineScript>
#include <QWebEnginePage>
#include <QWebEngineView>

#include <algorithm>

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
            showFullScreenHint();
        } else {
            exitFullScreen();
        }
    });

    // A Chrome-style transient hint so the user always knows how to get out of a
    // full-screen video (the web view swallows most keys, so this is easy to miss).
    m_fullScreenHint = new QLabel(tr("Press Esc to exit full screen"), this);
    m_fullScreenHint->setObjectName(u"fullScreenHint"_s);
    m_fullScreenHint->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_fullScreenHint->setStyleSheet(u"QLabel#fullScreenHint {"
                                    u"  background-color: rgba(0, 0, 0, 190);"
                                    u"  color: white;"
                                    u"  padding: 8px 18px;"
                                    u"  border-radius: 10px;"
                                    u"}"_s);
    m_fullScreenHint->hide();
    m_fullScreenHintTimer = new QTimer(this);
    m_fullScreenHintTimer->setSingleShot(true);
    m_fullScreenHintTimer->setInterval(3500);
    connect(m_fullScreenHintTimer, &QTimer::timeout, this, &PopupWindow::hideFullScreenHint);
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

void PopupWindow::showFullScreenHint()
{
    if (m_fullScreenHint == nullptr) {
        return;
    }
    m_fullScreenHint->setGraphicsEffect(nullptr); // clear any leftover fade
    positionFullScreenHint();
    m_fullScreenHint->show();
    m_fullScreenHint->raise();
    m_fullScreenHintTimer->start();
}

void PopupWindow::hideFullScreenHint()
{
    if (m_fullScreenHint == nullptr || !m_fullScreenHint->isVisible() ||
        m_fullScreenHint->graphicsEffect() != nullptr) {
        return; // hidden already, or a fade is already running
    }
    m_fullScreenHintTimer->stop();
    // The opacity effect is attached only for the fade and dropped afterwards (a
    // QGraphicsEffect left on a widget can force offscreen rendering that paints
    // it black on re-expose).
    auto* effect = new QGraphicsOpacityEffect(m_fullScreenHint);
    m_fullScreenHint->setGraphicsEffect(effect);
    auto* fade = new QPropertyAnimation(effect, "opacity", this);
    fade->setDuration(400);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);
    fade->setEasingCurve(QEasingCurve::InCubic);
    connect(fade, &QPropertyAnimation::finished, this, [this, effect] {
        if (m_fullScreenHint->graphicsEffect() == effect) {
            m_fullScreenHint->setGraphicsEffect(nullptr);
        }
        m_fullScreenHint->hide();
    });
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

void PopupWindow::positionFullScreenHint()
{
    if (m_fullScreenHint == nullptr) {
        return;
    }
    m_fullScreenHint->adjustSize();
    m_fullScreenHint->move(std::max(0, (width() - m_fullScreenHint->width()) / 2), 28);
}

void PopupWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_fullScreenHint != nullptr && m_fullScreenHint->isVisible()) {
        positionFullScreenHint();
    }
}

void PopupWindow::exitFullScreen()
{
    hideFullScreenHint();
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
