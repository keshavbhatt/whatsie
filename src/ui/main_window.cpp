#include "ui/main_window.h"

#include "core/navigation_policy.h"
#include "core/notifications/dnd_controller.h"
#include "core/settings/settings.h"
#include "core/theme/theme_service.h"
#include "core/zoom_policy.h"
#include "ui/about_dialog.h"
#include "ui/actions.h"
#include "ui/logging.h"
#include "ui/notification_hub.h"
#include "ui/settings_dialog.h"
#include "ui/theme_applier.h"
#include "ui/tray_controller.h"
#include "web/web_view.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QSessionManager>
#include <QWebEnginePage>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

namespace {
constexpr QSize kDefaultSize(1100, 720);
constexpr QSize kBaseMinimumSize(520, 440);
} // namespace

MainWindow::MainWindow(core::Settings& settings, core::ThemeService& theme, QWidget* parent)
    : QMainWindow(parent)
    , m_settings(settings)
    , m_theme(theme)
{
    setupUi();
    connectActions();
    connectWebView();
    restoreWindowState();
    applyMinimumSize();
    m_webView->loadWhatsApp();

    // Never veto a session logout/shutdown (W#66).
    connect(
        qApp, &QGuiApplication::commitDataRequest, this,
        [this](QSessionManager&) {
            m_quitting = true;
            saveWindowState();
        },
        Qt::DirectConnection);
}

MainWindow::~MainWindow() = default;

// ---- construction ----------------------------------------------------------

void MainWindow::setupUi()
{
    setWindowTitle(u"WhatsApp"_s);
    resize(kDefaultSize);

    m_themeApplier = new ThemeApplier(m_theme, this);
    m_actions = new Actions(this);
    m_dnd = new core::DndController(this);
    m_tray = new TrayController(m_settings, *m_dnd, *m_actions, this);
    setWindowIcon(m_tray->currentIcon());

    m_webView = new web::WebView(m_settings, this);
    setCentralWidget(m_webView);
    m_webView->page()->setBackgroundColor(m_theme.palette().color(QPalette::Window));
    connect(&m_theme, &core::ThemeService::effectiveSchemeChanged, this, [this](Qt::ColorScheme) {
        m_webView->page()->setBackgroundColor(m_theme.palette().color(QPalette::Window));
    });

    m_notifications = new NotificationHub(m_settings, *m_dnd, *m_tray, *m_webView, this);
    connect(m_notifications, &NotificationHub::activated, this, &MainWindow::showAndRaise);
}

void MainWindow::connectActions()
{
    connect(m_actions->showHide, &QAction::triggered, this, &MainWindow::toggleVisibility);
    connect(m_actions->newChat, &QAction::triggered, this, &MainWindow::promptNewChat);
    connect(m_actions->reload, &QAction::triggered, this, [this] { m_webView->reload(); });
    connect(m_actions->zoomIn, &QAction::triggered, this, [this] { m_webView->zoomStep(+1); });
    connect(m_actions->zoomOut, &QAction::triggered, this, [this] { m_webView->zoomStep(-1); });
    connect(m_actions->zoomReset, &QAction::triggered, this, [this] { m_webView->zoomReset(); });
    connect(m_actions->fullScreen, &QAction::toggled, this, &MainWindow::setFullScreenMode);
    connect(m_actions->settings, &QAction::triggered, this, &MainWindow::showSettings);
    connect(m_actions->about, &QAction::triggered, this, &MainWindow::showAbout);
    connect(m_actions->quit, &QAction::triggered, this, &MainWindow::quit);

    connect(m_tray, &TrayController::toggleRequested, this, &MainWindow::toggleVisibility);
    connect(m_tray, &TrayController::showRequested, this, &MainWindow::showAndRaise);
    connect(&m_settings, &core::Settings::zoomFactorChanged, this, [this](double) { applyMinimumSize(); });
}

void MainWindow::connectWebView()
{
    connect(m_webView, &web::WebView::unreadCountChanged, this, &MainWindow::handleUnread);
    connect(m_webView, &web::WebView::fullScreenRequested, this,
            [this](bool on) { m_actions->fullScreen->setChecked(on); });
    connect(m_webView, &web::WebView::renderProcessGaveUp, this, &MainWindow::handleRenderProcessGaveUp);
    connect(m_webView, &QWebEngineView::titleChanged, this,
            [this](const QString& title) { setWindowTitle(title.isEmpty() ? u"WhatsApp"_s : title); });
    connect(m_webView, &web::WebView::inAppPopupRequested, this, [](const QUrl& url) {
        // In-app pop-outs (calls) land in M3; until then just record it.
        qCWarning(lcUi) << "in-app popup not yet supported:" << url;
    });
}

// ---- lifecycle -------------------------------------------------------------

void MainWindow::start(bool startHidden)
{
    if (startHidden && m_tray->isAvailable()) {
        qCInfo(lcUi) << "starting hidden in tray";
        m_tray->setWindowVisible(false);
        return;
    }
    show();
}

void MainWindow::showAndRaise()
{
    show();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();
    activateWindow();
}

void MainWindow::toggleVisibility()
{
    if (isVisible() && !isMinimized() && isActiveWindow()) {
        if (m_tray->isAvailable()) {
            hide();
        }
        return;
    }
    showAndRaise();
}

void MainWindow::quit()
{
    m_quitting = true;
    saveWindowState();
    m_settings.sync();
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!m_quitting && m_settings.closeAction() == core::CloseAction::MinimizeToTray &&
        m_tray->isAvailable()) {
        hide();
        event->ignore();
        return;
    }
    saveWindowState();
    event->accept();
    QApplication::quit();
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    m_tray->setWindowVisible(true);
}

void MainWindow::hideEvent(QHideEvent* event)
{
    QMainWindow::hideEvent(event);
    m_tray->setWindowVisible(false);
}

void MainWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        updateZoomMode();
    }
}

// ---- window state ----------------------------------------------------------

void MainWindow::restoreWindowState()
{
    const QByteArray geometry = m_settings.windowGeometry();
    if (!geometry.isEmpty() && !restoreGeometry(geometry)) {
        qCWarning(lcUi) << "stored window geometry rejected, using defaults";
    }
    const QByteArray state = m_settings.windowState();
    if (!state.isEmpty()) {
        restoreState(state);
    }
    updateZoomMode();
}

void MainWindow::saveWindowState()
{
    if (isFullScreen()) {
        return; // never persist the full-screen state
    }
    m_settings.setWindowGeometry(saveGeometry());
    m_settings.setWindowState(saveState());
}

void MainWindow::updateZoomMode()
{
    m_webView->setZoomMode(isMaximized() || isFullScreen());
}

void MainWindow::applyMinimumSize()
{
    setMinimumSize(core::scaledMinimumSize(kBaseMinimumSize, m_settings.zoomFactor()));
}

void MainWindow::setFullScreenMode(bool on)
{
    if (on == isFullScreen()) {
        return;
    }
    if (on) {
        m_stateBeforeFullScreen = windowState();
        showFullScreen();
    } else {
        setWindowState(m_stateBeforeFullScreen & ~Qt::WindowFullScreen);
        if (!isVisible()) {
            show();
        }
    }
}

// ---- chat / dialogs --------------------------------------------------------

void MainWindow::openChat(const QString& target)
{
    const auto request = core::parseChatLink(target);
    if (!request) {
        qCWarning(lcUi) << "not a chat link or phone number:" << target;
        QMessageBox::warning(this, tr("Cannot open chat"),
                             tr("\"%1\" is not a phone number or WhatsApp link.").arg(target));
        return;
    }
    showAndRaise();
    m_webView->openChat(*request);
}

void MainWindow::promptNewChat()
{
    bool ok = false;
    const QString target =
        QInputDialog::getText(this, tr("New chat"), tr("Phone number with country code, or a wa.me link:"),
                              QLineEdit::Normal, QString(), &ok);
    if (ok && !target.trimmed().isEmpty()) {
        openChat(target);
    }
}

void MainWindow::showSettings()
{
    if (!m_settingsDialog) {
        m_settingsDialog = new SettingsDialog(m_settings, m_tray->isAvailable(), this);
        m_settingsDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_settingsDialog, &SettingsDialog::testNotificationRequested, m_notifications,
                &NotificationHub::sendTest);
    }
    m_settingsDialog->show();
    m_settingsDialog->raise();
    m_settingsDialog->activateWindow();
}

void MainWindow::showAbout()
{
    AboutDialog dialog(m_settings, m_webView->userAgent(), this);
    dialog.exec();
}

void MainWindow::handleUnread(int count)
{
    m_tray->setUnreadCount(count);
    setWindowIcon(m_tray->currentIcon());
}

void MainWindow::handleRenderProcessGaveUp()
{
    showAndRaise();
    const auto choice = QMessageBox::critical(
        this, tr("WhatsApp Web keeps crashing"),
        tr("The page's render process has crashed several times in a row.\n\n"
           "Reload to try again, or quit. If this keeps happening, use Settings → Advanced to find the log."),
        QMessageBox::Retry | QMessageBox::Close, QMessageBox::Retry);
    if (choice == QMessageBox::Retry) {
        m_webView->reload();
    } else {
        quit();
    }
}

} // namespace whatsie::ui
