#include "web/web_view.h"

#include "core/settings/settings.h"
#include "core/theme/theme_service.h"
#include "core/unread_badge.h"
#include "core/zoom_policy.h"
#include "web/bridge.h"
#include "web/clipboard_fix.h"
#include "web/file_drop.h"
#include "web/logging.h"
#include "web/permission_controller.h"
#include "web/popup_window.h"
#include "web/web_page.h"
#include "web/web_profile.h"

#include <QApplication>
#include <QChildEvent>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFutureWatcher>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QNetworkInformation>
#include <QTimer>
#include <QUrl>
#include <QWebChannel>
#include <QWebEngineContextMenuRequest>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWheelEvent>
#include <QtConcurrent>

using namespace Qt::StringLiterals;

namespace whatsie::web {

namespace {
const QUrl kWhatsAppUrl(u"https://web.whatsapp.com/"_s);
} // namespace

WebView::WebView(core::Settings& settings, core::ThemeService& theme, QWidget* parent)
    : QWebEngineView(parent)
    , m_settings(settings)
    , m_profile(new WebProfile(settings, this))
    , m_page(new WebPage(*m_profile, this))
    , m_permissions(new PermissionController(this))
{
    m_clock.start();
    m_page->setHostWidget(this);
    setPage(m_page);
    applyZoom();

    // One channel per page exposing the profile's bridge object (ADR-006).
    auto* channel = new QWebChannel(m_page);
    channel->registerObject(u"bridge"_s, &m_profile->bridge());
    m_page->setWebChannel(channel, QWebEngineScript::MainWorld);

    m_permissions->attach(*m_page);
    connect(m_permissions, &PermissionController::promptRequested, this, &WebView::permissionPromptRequested);
    connect(m_page, &QWebEnginePage::desktopMediaRequested, this, &WebView::desktopMediaRequested);
    // Call pop-out windows use a separate page on the same profile; give it the
    // same permission answering and screen-share handling as the main view.
    connect(m_page, &WebPage::popupOpened, this, &WebView::wirePopup);

    connect(&m_settings, &core::Settings::zoomFactorChanged, this, [this](double) { applyZoom(); });
    connect(&m_settings, &core::Settings::zoomFactorMaximizedChanged, this, [this](double) { applyZoom(); });
    connect(&m_settings, &core::Settings::mutedChanged, m_page, &QWebEnginePage::setAudioMuted);
    m_page->setAudioMuted(m_settings.muted());

    connect(m_page, &QWebEnginePage::titleChanged, this, &WebView::handleTitleChanged);
    connect(m_page, &QWebEnginePage::renderProcessTerminated, this, &WebView::handleRenderProcessTerminated);
    connect(m_page, &QWebEnginePage::loadFinished, this, [this](bool ok) {
        if (ok) {
            m_crashPolicy.onLoadSucceeded();
            applyBlurLive();
            applyThemeLive();
        }
    });

    // Connection watchdog (FEATURES S13): the injected script reports up/down;
    // a timer drives the reload decision while down.
    m_watchdogTimer = new QTimer(this);
    m_watchdogTimer->setInterval(5000);
    connect(m_watchdogTimer, &QTimer::timeout, this, &WebView::checkWatchdog);
    connect(&m_profile->bridge(), &Bridge::connectionStateChanged, this, &WebView::handleConnectionChanged);

    // Network came back (FEATURES S14): force a reload attempt if we are down.
    if (QNetworkInformation::loadDefaultBackend()) {
        connect(QNetworkInformation::instance(), &QNetworkInformation::reachabilityChanged, this,
                [this](QNetworkInformation::Reachability r) {
                    if (r == QNetworkInformation::Reachability::Online) {
                        m_watchdog.networkReturned(std::chrono::milliseconds(m_clock.elapsed()));
                        checkWatchdog();
                    }
                });
    }

    connect(&m_settings, &core::Settings::messageBlurLevelChanged, this, [this](int) { applyBlurLive(); });
    connect(&m_settings, &core::Settings::themeChanged, this, [this](core::Theme) { applyThemeLive(); });
    connect(m_page, &QWebEnginePage::fullScreenRequested, this, [this](QWebEngineFullScreenRequest request) {
        request.accept();
        Q_EMIT fullScreenRequested(request.toggleOn());
    });
    // Queued so ui::ThemeApplier (a direct listener) has updated QStyleHints first.
    connect(
        &theme, &core::ThemeService::effectiveSchemeChanged, this,
        [this](Qt::ColorScheme) {
            refreshColorScheme();
            applyThemeLive(); // repaint WhatsApp when the OS scheme changes in System mode
        },
        Qt::QueuedConnection);
}

// Qt WebEngine copies QStyleHints::colorScheme() into Blink's preferences only
// when web settings are applied. Toggling an attribute re-applies them, which
// updates prefers-color-scheme in the running page — WhatsApp then switches
// its theme instantly (ADR-020).
void WebView::refreshColorScheme()
{
    QWebEngineSettings* s = m_page->settings();
    const bool smooth = s->testAttribute(QWebEngineSettings::ScrollAnimatorEnabled);
    s->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, !smooth);
    s->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, smooth);
    qCInfo(lcWeb) << "colour scheme pushed to the page";
}

void WebView::wirePopup(QWidget* window)
{
    auto* popup = qobject_cast<PopupWindow*>(window);
    if (popup == nullptr) {
        return;
    }
    QWebEnginePage* page = popup->page();
    m_permissions->attach(*page);
    connect(page, &QWebEnginePage::desktopMediaRequested, this, &WebView::desktopMediaRequested);
    qCInfo(lcWeb) << "wired pop-out window for media";
}

WebView::~WebView()
{
    // Qt WebEngine requires the profile to outlive every page created from it.
    // The profile and pages are siblings under this view, and QObject would
    // otherwise destroy the profile first ("Release of profile requested but
    // WebEnginePage still not deleted"). Detach and delete the page (and any
    // open pop-up windows, whose pages also use the profile) up front.
    setPage(nullptr);
    for (PopupWindow* popup : findChildren<PopupWindow*>(Qt::FindDirectChildrenOnly)) {
        delete popup;
    }
    delete m_page;
    m_page = nullptr;
    delete m_permissions;
    m_permissions = nullptr;
    // m_profile (still a child) is destroyed last, by the QObject base dtor.
}

void WebView::handleConnectionChanged(bool up)
{
    Q_EMIT connectionChanged(up);
    m_watchdog.setConnected(up, std::chrono::milliseconds(m_clock.elapsed()));
    if (up) {
        m_watchdogTimer->stop();
    } else {
        m_watchdogTimer->start();
    }
}

void WebView::checkWatchdog()
{
    const std::chrono::milliseconds now(m_clock.elapsed());
    if (m_watchdog.shouldReload(now)) {
        qCInfo(lcWeb) << "connection down; reloading (attempt" << m_watchdog.reloadsThisEpisode() + 1 << ")";
        m_watchdog.noteReload(now);
        reload();
    }
}

// FEATURES M6: attach files dragged onto the window. We handle the drop
// ourselves and inject real File objects via file-drop.js, which works where
// Chromium's native drop cannot read the paths (Wayland / Flatpak, Y#32).
bool WebView::maybeHandleDrop(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched)
    const QEvent::Type t = event->type();
    if (t != QEvent::DragEnter && t != QEvent::DragMove && t != QEvent::Drop) {
        return false;
    }
    auto* drop = static_cast<QDropEvent*>(event);
    const QMimeData* mime = drop->mimeData();
    if (mime == nullptr || !mime->hasUrls()) {
        return false; // not a file drop — let Chromium handle it
    }
    QStringList paths;
    for (const QUrl& url : mime->urls()) {
        if (url.isLocalFile()) {
            paths << url.toLocalFile();
        }
    }
    if (paths.isEmpty()) {
        return false;
    }
    if (t == QEvent::Drop) {
        drop->setDropAction(Qt::CopyAction);
        drop->accept();
        handleFileDrop(paths);
    } else {
        auto* move = static_cast<QDragMoveEvent*>(event);
        move->setDropAction(Qt::CopyAction);
        move->accept();
    }
    return true;
}

void WebView::handleFileDrop(const QStringList& paths)
{
    qCInfo(lcWeb) << "file drop:" << paths.size() << "file(s)";
    auto* watcher = new QFutureWatcher<DropOutcome>(this);
    connect(watcher, &QFutureWatcher<DropOutcome>::finished, this, [this, watcher] {
        const DropOutcome outcome = watcher->result();
        watcher->deleteLater();
        if (!outcome.skipped.isEmpty()) {
            qCWarning(lcWeb) << "drop skipped:" << outcome.skipped;
        }
        if (outcome.files.isEmpty()) {
            return;
        }
        // A JSON array is valid JavaScript (base64/strings are properly escaped),
        // so pass it straight as the call argument.
        const QString arrayLiteral =
            QString::fromUtf8(QJsonDocument(outcome.files).toJson(QJsonDocument::Compact));
        m_page->runJavaScript(
            u"window.__whatsieDropFiles && window.__whatsieDropFiles(%1)"_s.arg(arrayLiteral),
            QWebEngineScript::MainWorld);
    });
    watcher->setFuture(QtConcurrent::run([paths] { return buildDropPayload(paths, kMaxDropBytesPerFile); }));
}

void WebView::applyBlurLive()
{
    m_page->runJavaScript(
        u"window.__whatsieSetBlur && window.__whatsieSetBlur(%1)"_s.arg(m_settings.messageBlurLevel()),
        QWebEngineScript::MainWorld);
}

void WebView::applyThemeLive()
{
    // WhatsApp is themed at the page level because the platform theme overrides
    // QStyleHints::setColorScheme before it reaches Blink (ADR-026).
    QString mode;
    switch (m_settings.theme()) {
    case core::Theme::Light:
        mode = u"light"_s;
        break;
    case core::Theme::Dark:
        mode = u"dark"_s;
        break;
    case core::Theme::System:
        mode = u"system"_s;
        break;
    }
    m_page->runJavaScript(u"window.__whatsieSetTheme && window.__whatsieSetTheme('%1')"_s.arg(mode),
                          QWebEngineScript::MainWorld);
    qCDebug(lcWeb) << "theme pushed to page:" << mode;
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

// Input arrives at Chromium's child render widget, not at the view, so the
// filter is installed on every child.
void WebView::childEvent(QChildEvent* event)
{
    QWebEngineView::childEvent(event);
    if (event->type() == QEvent::ChildAdded && event->child() != nullptr) {
        event->child()->installEventFilter(this);
    }
}

bool WebView::eventFilter(QObject* watched, QEvent* event)
{
    if (maybeHandleDrop(watched, event)) {
        return true;
    }
    if (event->type() == QEvent::Wheel) {
        auto* wheel = static_cast<QWheelEvent*>(event);
        if (wheel->modifiers().testFlag(Qt::ControlModifier)) {
            const int delta = wheel->angleDelta().y();
            if (delta != 0) {
                zoomStep(delta > 0 ? 1 : -1);
            }
            return true;
        }
    } else if (event->type() == QEvent::KeyPress) {
        // FEATURES M7: make clipboard images pasteable before Chromium reads them.
        auto* key = static_cast<QKeyEvent*>(event);
        if (key->matches(QKeySequence::Paste)) {
            ensureClipboardImageIsPng(*QApplication::clipboard());
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
