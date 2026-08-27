#include "web/web_view.h"

#include "core/settings/settings.h"
#include "core/unread_badge.h"
#include "core/zoom_policy.h"
#include "web/logging.h"
#include "web/web_page.h"
#include "web/web_profile.h"

#include <QChildEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QTimer>
#include <QUrl>
#include <QWebEngineContextMenuRequest>
#include <QWebEngineFullScreenRequest>
#include <QWheelEvent>

using namespace Qt::StringLiterals;

namespace whatsie::web {

namespace {
const QUrl kWhatsAppUrl(u"https://web.whatsapp.com/"_s);
} // namespace

WebView::WebView(core::Settings& settings, QWidget* parent)
    : QWebEngineView(parent)
    , m_settings(settings)
    , m_profile(new WebProfile(settings, this))
    , m_page(new WebPage(*m_profile, this))
{
    m_clock.start();
    setPage(m_page);
    applyZoom();

    connect(&m_settings, &core::Settings::zoomFactorChanged, this, [this](double) { applyZoom(); });
    connect(&m_settings, &core::Settings::zoomFactorMaximizedChanged, this, [this](double) { applyZoom(); });
    connect(m_page, &QWebEnginePage::titleChanged, this, &WebView::handleTitleChanged);
    connect(m_page, &QWebEnginePage::renderProcessTerminated, this, &WebView::handleRenderProcessTerminated);
    connect(m_page, &QWebEnginePage::loadFinished, this, [this](bool ok) {
        if (ok) {
            m_crashPolicy.onLoadSucceeded();
        }
    });
    connect(m_page, &QWebEnginePage::fullScreenRequested, this, [this](QWebEngineFullScreenRequest request) {
        request.accept();
        Q_EMIT fullScreenRequested(request.toggleOn());
    });
    connect(m_page, &WebPage::inAppPopupRequested, this, &WebView::inAppPopupRequested);
}

void WebView::loadWhatsApp()
{
    if (url() == kWhatsAppUrl) {
        return;
    }
    qCInfo(lcWeb) << "loading" << kWhatsAppUrl;
    load(kWhatsAppUrl);
}

void WebView::openChat(const core::NewChatRequest& request)
{
    const QUrl target = core::newChatUrl(request);
    qCInfo(lcWeb) << "open chat" << target.toString(QUrl::RemoveQuery) << "phone=" << request.phone;
    load(target);
}

QString WebView::userAgent() const
{
    return m_profile->httpUserAgent();
}

// ---- zoom ------------------------------------------------------------------

void WebView::setZoomMode(bool maximized)
{
    if (m_maximizedMode == maximized) {
        return;
    }
    m_maximizedMode = maximized;
    applyZoom();
}

void WebView::applyZoom()
{
    setZoomFactor(m_maximizedMode ? m_settings.zoomFactorMaximized() : m_settings.zoomFactor());
}

void WebView::zoomStep(int direction)
{
    const double current = zoomFactor();
    const double next = direction > 0 ? core::zoomIn(current) : core::zoomOut(current);
    if (m_maximizedMode) {
        m_settings.setZoomFactorMaximized(next);
    } else {
        m_settings.setZoomFactor(next);
    }
    applyZoom();
}

void WebView::zoomReset()
{
    if (m_maximizedMode) {
        m_settings.setZoomFactorMaximized(core::kDefaultZoom);
    } else {
        m_settings.setZoomFactor(core::kDefaultZoom);
    }
    applyZoom();
}

// Ctrl+wheel arrives at Chromium's child render widget, not at the view, so
// the filter is installed on every child (W: "Ctrl+wheel cannot be blocked at
// the view level").
void WebView::childEvent(QChildEvent* event)
{
    QWebEngineView::childEvent(event);
    if (event->type() == QEvent::ChildAdded && event->child() != nullptr) {
        event->child()->installEventFilter(this);
    }
}

bool WebView::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Wheel) {
        auto* wheel = static_cast<QWheelEvent*>(event);
        if (wheel->modifiers().testFlag(Qt::ControlModifier)) {
            const int delta = wheel->angleDelta().y();
            if (delta != 0) {
                zoomStep(delta > 0 ? 1 : -1);
            }
            return true;
        }
    }
    return QWebEngineView::eventFilter(watched, event);
}

// ---- unread ----------------------------------------------------------------

void WebView::handleTitleChanged(const QString& title)
{
    const int count = core::unreadCountFromTitle(title);
    if (count == m_unread) {
        return;
    }
    m_unread = count;
    Q_EMIT unreadCountChanged(count);
}

// ---- crash recovery (FEATURES S12) ----------------------------------------

void WebView::handleRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status,
                                            int exitCode)
{
    if (status == QWebEnginePage::NormalTerminationStatus) {
        return;
    }
    qCWarning(lcWeb) << "render process terminated, status" << status << "exit code" << exitCode;
    const auto decision = m_crashPolicy.onCrash(std::chrono::milliseconds(m_clock.elapsed()));
    if (!decision.reload) {
        qCCritical(lcWeb) << "render process keeps crashing; asking the user";
        Q_EMIT renderProcessGaveUp();
        return;
    }
    qCInfo(lcWeb) << "reloading in" << decision.delay.count() << "ms";
    QTimer::singleShot(decision.delay, this, [this] { reload(); });
}

// ---- context menu (FEATURES M13) ------------------------------------------

void WebView::contextMenuEvent(QContextMenuEvent* event)
{
    const QWebEngineContextMenuRequest* request = lastContextMenuRequest();
    if (request == nullptr) {
        return;
    }
    const bool editable = request->isContentEditable();
    const bool hasSelection = !request->selectedText().isEmpty();
    const bool onImage = request->mediaType() == QWebEngineContextMenuRequest::MediaTypeImage;
    const bool onLink = !request->linkUrl().isEmpty();
    if (!editable && !hasSelection && !onImage && !onLink) {
        return; // WhatsApp Web has its own menus; nothing useful to add here.
    }

    auto* menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    if (editable) {
        menu->addAction(pageAction(QWebEnginePage::Undo));
        menu->addAction(pageAction(QWebEnginePage::Redo));
        menu->addSeparator();
        menu->addAction(pageAction(QWebEnginePage::Cut));
    }
    if (hasSelection || editable) {
        menu->addAction(pageAction(QWebEnginePage::Copy));
    }
    if (editable) {
        menu->addAction(pageAction(QWebEnginePage::Paste));
        menu->addAction(pageAction(QWebEnginePage::SelectAll));
    }
    if (onLink) {
        menu->addAction(pageAction(QWebEnginePage::CopyLinkToClipboard));
    }
    if (onImage) {
        menu->addAction(pageAction(QWebEnginePage::CopyImageToClipboard));
    }
    menu->popup(event->globalPos());
}

} // namespace whatsie::web
