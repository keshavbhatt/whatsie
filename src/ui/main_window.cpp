#include "ui/main_window.h"

#include "core/app_lock.h"
#include "core/navigation_policy.h"
#include "core/notifications/dnd_controller.h"
#include "core/notifications/notification_service.h"
#include "core/settings/settings.h"
#include "core/theme/theme_service.h"
#include "core/zoom_policy.h"
#include "platform/autostart.h"
#include "ui/about_dialog.h"
#include "ui/actions.h"
#include "ui/downloads_hub.h"
#include "ui/lock_screen.h"
#include "ui/logging.h"
#include "ui/notification_hub.h"
#include "ui/permission_prompt.h"
#include "ui/proxy_auth_dialog.h"
#include "ui/screen_picker_dialog.h"
#include "ui/settings_dialog.h"
#include "ui/shortcuts_dialog.h"
#include "ui/theme_applier.h"
#include "ui/tray_controller.h"
#include "web/permission_controller.h"
#include "web/web_profile.h"
#include "web/web_view.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QApplication>
#include <QAuthenticator>
#include <QCloseEvent>
#include <QFile>
#include <QGuiApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QSessionManager>
#include <QShortcut>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTimer>
#include <QWebEngineDesktopMediaRequest>
#include <QWebEnginePage>
#include <QWebEnginePermission>

#include <chrono>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

namespace {
constexpr QSize kDefaultSize(1100, 720);
constexpr QSize kBaseMinimumSize(520, 440);

// WhatsApp Web's own startup/loading background. The web page is painted with
// this before its content renders, so the first paint matches the loading
// screen and there is no visible flash from the native chrome colour.
QColor whatsappPageBackground(bool dark)
{
    return dark ? QColor(0x11, 0x1B, 0x21) : QColor(0xF0, 0xF2, 0xF5);
}
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

    m_webView = new web::WebView(m_settings, m_theme, this);
    m_lockScreen = new LockScreen(this);
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(m_webView);    // index 0
    m_stack->addWidget(m_lockScreen); // index 1
    setCentralWidget(m_stack);
    m_webView->page()->setBackgroundColor(whatsappPageBackground(m_theme.isDark()));
    connect(&m_theme, &core::ThemeService::effectiveSchemeChanged, this, [this](Qt::ColorScheme) {
        m_webView->page()->setBackgroundColor(whatsappPageBackground(m_theme.isDark()));
    });

    m_notifications = new NotificationHub(m_settings, *m_dnd, *m_tray, *m_webView, this);
    connect(m_notifications, &NotificationHub::activated, this, &MainWindow::showAndRaise);
    m_downloads = new DownloadsHub(m_settings, m_notifications->service(), *m_webView, this, this);

    syncAutostart();
    setupLock();
}

void MainWindow::connectActions()
{
    connect(m_actions->showHide, &QAction::triggered, this, &MainWindow::toggleVisibility);
    connect(m_actions->newChat, &QAction::triggered, this, &MainWindow::promptNewChat);
    connect(m_actions->reload, &QAction::triggered, this, [this] { m_webView->reload(); });
    connect(m_actions->downloads, &QAction::triggered, m_downloads, &DownloadsHub::showWindow);
    m_actions->mute->setChecked(m_settings.muted());
    connect(m_actions->mute, &QAction::toggled, this, [this](bool on) { m_settings.setMuted(on); });
    connect(&m_settings, &core::Settings::mutedChanged, m_actions->mute, &QAction::setChecked);
    m_actions->blurMessages->setChecked(m_settings.messageBlurLevel() > 0);
    connect(m_actions->blurMessages, &QAction::toggled, this,
            [this](bool on) { m_settings.setMessageBlurLevel(on ? 2 : 0); });
    connect(m_actions->toggleTheme, &QAction::triggered, this, &MainWindow::toggleTheme);
    connect(m_actions->lock, &QAction::triggered, this, &MainWindow::requestLock);
    connect(&m_settings, &core::Settings::messageBlurLevelChanged, this,
            [this](int level) { m_actions->blurMessages->setChecked(level > 0); });
    connect(m_actions->zoomIn, &QAction::triggered, this, [this] { m_webView->zoomStep(+1); });
    connect(m_actions->zoomOut, &QAction::triggered, this, [this] { m_webView->zoomStep(-1); });
    connect(m_actions->zoomReset, &QAction::triggered, this, [this] { m_webView->zoomReset(); });
    connect(m_actions->fullScreen, &QAction::toggled, this, &MainWindow::setFullScreenMode);
    // Esc always leaves full screen, even if WhatsApp's own video control is
    // unreachable — only active while full screen so it never eats the page's
    // own Esc (closing chats/search) otherwise.
    m_exitFullScreen = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    m_exitFullScreen->setContext(Qt::WindowShortcut);
    m_exitFullScreen->setEnabled(false);
    connect(m_exitFullScreen, &QShortcut::activated, this,
            [this] { m_actions->fullScreen->setChecked(false); });
    connect(m_actions->settings, &QAction::triggered, this, &MainWindow::showSettings);
    connect(m_actions->shortcuts, &QAction::triggered, this, &MainWindow::showShortcuts);
    connect(m_actions->about, &QAction::triggered, this, &MainWindow::showAbout);
    connect(&m_settings, &core::Settings::autostartChanged, this, [this] { syncAutostart(); });
    connect(&m_settings, &core::Settings::startMinimizedChanged, this, [this] { syncAutostart(); });
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
    connect(m_webView, &web::WebView::permissionPromptRequested, this,
            [this](const QWebEnginePermission& permission) {
                askPermission(this, permission, [this, permission](bool allow) {
                    m_webView->permissions().answer(permission, allow);
                });
            });
    connect(m_webView, &web::WebView::desktopMediaRequested, this,
            [this](QWebEngineDesktopMediaRequest request) { handleDesktopMediaRequest(std::move(request)); });
    connect(m_webView, &web::WebView::connectionChanged, m_tray, &TrayController::setConnected);
    connect(m_webView, &web::WebView::settingsRequested, this, &MainWindow::showSettings);
    connect(m_webView, &web::WebView::proxyAuthenticationRequired, this, &MainWindow::handleProxyAuth);
}

// Screen sharing (getDisplayMedia). On Wayland the PipeWire desktop portal
// presents its own native picker (screen, window, region) and performs the
// capture; a second in-app picker is redundant and — because the portal can
// invalidate the request out from under it — was the source of a crash when
// the user dismissed the portal and then clicked our dialog. So on Wayland we
// hand the request to the portal by selecting the primary screen and show no
// dialog. On X11 there is no portal, so our own picker is the only UI.
void MainWindow::handleDesktopMediaRequest(QWebEngineDesktopMediaRequest request)
{
    const bool wayland = QGuiApplication::platformName() == QLatin1StringView("wayland");
    qCInfo(lcUi) << "desktopMediaRequested platform=" << QGuiApplication::platformName();
    if (wayland) {
        QAbstractItemModel* screens = request.screensModel();
        if (screens != nullptr && screens->rowCount() > 0) {
            request.selectScreen(screens->index(0, 0)); // portal decides the actual source
        } else if (request.windowsModel() != nullptr && request.windowsModel()->rowCount() > 0) {
            request.selectWindow(request.windowsModel()->index(0, 0));
        } else {
            request.cancel();
        }
        return;
    }
    auto* picker = new ScreenPickerDialog(std::move(request), this);
    picker->setAttribute(Qt::WA_DeleteOnClose);
    picker->open();
}

// ---- lifecycle -------------------------------------------------------------

void MainWindow::start(bool startHidden)
{
    if (m_settings.lockOnStart()) {
        lock(); // no-op without a passcode
    }
    if (startHidden && m_tray->isAvailable()) {
        qCInfo(lcUi) << "starting hidden in tray";
        m_tray->setWindowVisible(false);
        return;
    }
    show();
    if (m_locked) {
        m_lockScreen->reset();
    }
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
            if (m_settings.lockOnHide()) {
                lock();
            }
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
        if (m_settings.lockOnHide()) {
            lock();
        }
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
        // Still keep the page and the Esc hatch in sync even if the window is
        // already in the target state (e.g. a page-driven request while there).
        m_exitFullScreen->setEnabled(on);
        if (!on) {
            m_webView->exitPageFullScreen();
        }
        return;
    }
    if (on) {
        m_stateBeforeFullScreen = windowState();
        showFullScreen();
        m_exitFullScreen->setEnabled(true);
    } else {
        // Restore explicitly (setWindowState clearing the flag is unreliable on
        // Wayland) and make sure the page leaves its own HTML full screen too,
        // so an exit started on either side can't leave the other stuck.
        m_exitFullScreen->setEnabled(false);
        if (m_stateBeforeFullScreen & Qt::WindowMaximized) {
            showMaximized();
        } else {
            showNormal();
        }
        m_webView->exitPageFullScreen();
    }
}

// ---- chat / dialogs --------------------------------------------------------

void MainWindow::openChat(const QString& target)
{
    if (m_locked) {
        showAndRaise();
        return;
    }
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
    if (m_locked) {
        showAndRaise();
        return;
    }
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
    if (m_locked) {
        showAndRaise();
        return;
    }
    if (!m_settingsDialog) {
        const StoragePaths storage{m_webView->profile().cachePath(),
                                   m_webView->profile().persistentStoragePath()};
        m_settingsDialog =
            new SettingsDialog(m_settings, m_tray->isAvailable(), storage, m_webView->profile(), this);
        m_settingsDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_settingsDialog, &SettingsDialog::testNotificationRequested, m_notifications,
                &NotificationHub::sendTest);
        connect(m_settingsDialog, &SettingsDialog::clearCacheRequested, this, &MainWindow::clearCache);
        connect(m_settingsDialog, &SettingsDialog::clearSessionRequested, this,
                &MainWindow::confirmClearSession);
    }
    m_settingsDialog->show();
    m_settingsDialog->raise();
    m_settingsDialog->activateWindow();
}

void MainWindow::showShortcuts()
{
    ShortcutsDialog dialog(*m_actions, this);
    dialog.exec();
}

void MainWindow::handleProxyAuth(const QString& proxyHost, QAuthenticator* authenticator)
{
    ProxyAuthDialog dialog(proxyHost, m_settings.proxyUser(), this);
    if (dialog.exec() != QDialog::Accepted) {
        return; // leave the authenticator empty; the request fails
    }
    authenticator->setUser(dialog.user());
    authenticator->setPassword(dialog.password());
    m_settings.setProxyUser(dialog.user());         // remembered
    m_settings.setProxyPassword(dialog.password()); // session only
}

// ---- app lock (FEATURES P1, ADR-015) --------------------------------------

void MainWindow::setupLock()
{
    connect(m_lockScreen, &LockScreen::unlockRequested, this, &MainWindow::attemptUnlock);
    connect(m_lockScreen, &LockScreen::dismissed, this, &MainWindow::finalizeUnlock);
    m_idleTimer = new QTimer(this);
    m_idleTimer->setSingleShot(true);
    connect(m_idleTimer, &QTimer::timeout, this, &MainWindow::lock);
    m_throttleTimer = new QTimer(this);
    m_throttleTimer->setInterval(1000);
    connect(m_throttleTimer, &QTimer::timeout, this, &MainWindow::tickThrottle);
    connect(&m_settings, &core::Settings::lockConfigChanged, this, &MainWindow::updateIdleTimer);
    qApp->installEventFilter(this); // reset the idle timer on any input
    updateIdleTimer();
}

void MainWindow::requestLock()
{
    if (!m_settings.hasPasscode()) {
        const auto choice = QMessageBox::question(
            this, tr("Screen lock"), tr("Set a passcode first to use the lock.\n\nOpen Settings now?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (choice == QMessageBox::Yes) {
            showSettings();
        }
        return;
    }
    lock();
}

void MainWindow::lock()
{
    if (!m_settings.hasPasscode()) {
        return;
    }
    if (m_locked && !m_unlocking) {
        return; // already fully locked
    }
    // If an unlock animation is mid-flight, cancel it so we stay locked — see the
    // m_unlocking guard in finalizeUnlock().
    m_unlocking = false;
    m_locked = true;
    m_failedAttempts = 0;
    m_lockScreen->reset();
    m_stack->setCurrentWidget(m_lockScreen);
    m_notifications->service().setLockSuppressed(true);
    // Cover every window: close the auxiliary ones and any popped-out call.
    if (m_settingsDialog) {
        m_settingsDialog->close();
    }
    m_downloads->setSuppressed(true);
    m_webView->closePopups();
    // Close any modal dialog (About, New chat, proxy prompt, …) sitting above the
    // lock screen; an idle lock can fire inside its nested event loop and would
    // otherwise leave it on top, grabbing input, with the passcode field
    // unreachable and the dialog still operable "while locked".
    while (QWidget* modal = QApplication::activeModalWidget()) {
        modal->close();
        if (QApplication::activeModalWidget() == modal) {
            break; // a modal that refuses to close — do not spin
        }
    }
    m_idleTimer->stop();
    qCInfo(lcUi) << "app locked";
}

void MainWindow::unlock()
{
    if (!m_locked || m_unlocking) {
        return;
    }
    // Play the lock screen's dismissal, then reveal the app in finalizeUnlock()
    // when it signals dismissed().
    m_unlocking = true;
    m_lockScreen->playDismiss();
}

void MainWindow::finalizeUnlock()
{
    if (!m_unlocking) {
        return; // the unlock was cancelled (e.g. re-locked mid-animation)
    }
    m_locked = false;
    m_unlocking = false;
    m_failedAttempts = 0;
    m_throttleRemaining = 0;
    m_throttleTimer->stop();
    m_stack->setCurrentWidget(m_webView);
    m_notifications->service().setLockSuppressed(false);
    m_downloads->setSuppressed(false);
    updateIdleTimer();
    qCInfo(lcUi) << "app unlocked";
}

void MainWindow::attemptUnlock(const QString& passcode)
{
    if (m_throttleRemaining > 0) {
        return; // still locked out
    }
    if (core::verifyPasscode(passcode, m_settings.passcodeRecord())) {
        unlock();
        return;
    }
    ++m_failedAttempts;
    const auto lockout = core::lockoutDuration(m_failedAttempts);
    if (lockout.count() > 0) {
        m_throttleRemaining =
            static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(lockout).count());
        m_lockScreen->showThrottle(m_throttleRemaining);
        m_throttleTimer->start();
    } else {
        m_lockScreen->showError(tr("Incorrect passcode."));
    }
}

void MainWindow::tickThrottle()
{
    if (--m_throttleRemaining <= 0) {
        m_throttleRemaining = 0;
        m_throttleTimer->stop();
        m_lockScreen->showThrottle(0); // re-enable entry
    } else {
        m_lockScreen->showThrottle(m_throttleRemaining);
    }
}

void MainWindow::updateIdleTimer()
{
    const int minutes = m_settings.lockIdleMinutes();
    if (m_settings.hasPasscode() && minutes > 0 && !m_locked) {
        m_idleTimer->start(minutes * 60 * 1000);
    } else {
        m_idleTimer->stop();
    }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::KeyPress:
    case QEvent::Wheel:
    case QEvent::TouchBegin:
        if (!m_locked && m_idleTimer != nullptr && m_idleTimer->isActive()) {
            m_idleTimer->start(); // restart with the configured interval
        }
        break;
    default:
        break;
    }
    return QMainWindow::eventFilter(watched, event);
}

// FEATURES A1: flip between Light and Dark based on what is on screen now, so it
// works even from "Follow system" (old whatsie's Ctrl+T quick action).
void MainWindow::toggleTheme()
{
    m_settings.setTheme(m_theme.isDark() ? core::Theme::Light : core::Theme::Dark);
}

void MainWindow::showAbout()
{
    AboutDialog dialog(m_settings, m_webView->userAgent(), this);
    dialog.exec();
}

void MainWindow::syncAutostart()
{
    if (!platform::setAutostartEnabled(m_settings.autostart(), m_settings.startMinimized())) {
        if (m_settings.autostart()) {
            qCWarning(lcUi) << "could not write autostart entry";
        }
    }
}

void MainWindow::handleUnread(int count)
{
    m_tray->setUnreadCount(count);
    setWindowIcon(m_tray->currentIcon());
}

void MainWindow::handleRenderProcessGaveUp()
{
    showAndRaise();
    const auto choice =
        QMessageBox::critical(this, tr("WhatsApp Web keeps crashing"),
                              tr("The page's render process has crashed several times in a row.\n\n"
                                 "Reload to try again, or quit. If this keeps happening, use Settings → "
                                 "Privacy & Advanced to find the log."),
                              QMessageBox::Retry | QMessageBox::Close, QMessageBox::Retry);
    if (choice == QMessageBox::Retry) {
        m_webView->reload();
    } else {
        quit();
    }
}

// ---- privacy & storage (FEATURES M1, P5) ----------------------------------

void MainWindow::clearCache()
{
    m_webView->profile().clearHttpCache();
    qCInfo(lcUi) << "http cache cleared";
}

void MainWindow::confirmClearSession()
{
    const auto choice = QMessageBox::warning(
        this, tr("Log out and clear session?"),
        tr("This removes the WhatsApp session and all cached data from this computer. "
           "Whatsie will quit; when you start it again you link your phone with the QR code.\n\n"
           "Your downloaded files are kept."),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (choice != QMessageBox::Yes) {
        return;
    }
    // Deleting the profile while the engine runs is unsafe: leave a marker
    // that Application honours on the next start (ADR-011: explicit action).
    const QString marker =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/clear-session"_s;
    QFile file(marker);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Cannot clear session"), tr("Could not write %1.").arg(marker));
        return;
    }
    file.close();
    qCInfo(lcUi) << "session clear scheduled for next start";
    quit();
}

} // namespace whatsie::ui
