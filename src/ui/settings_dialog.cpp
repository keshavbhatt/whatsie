#include "ui/settings_dialog.h"

#include "core/app_lock.h"
#include "core/downloads/file_naming.h"
#include "core/log_sink.h"
#include "core/settings/settings.h"
#include "core/spellcheck.h"
#include "core/storage_policy.h"
#include "core/zoom_policy.h"
#include "platform/file_manager.h"
#include "ui/passcode_dialog.h"
#include "ui/permission_list.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFutureWatcher>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTabWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QtEnvironmentVariables>

using namespace Qt::StringLiterals;
using whatsie::core::CloseAction;
using whatsie::core::HardwareAcceleration;
using whatsie::core::ProxyMode;
using whatsie::core::ProxyType;
using whatsie::core::Theme;

namespace whatsie::ui {

SettingsDialog::SettingsDialog(core::Settings& settings, bool trayAvailable, StoragePaths storage,
                               QWebEngineProfile& profile, QWidget* parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_trayAvailable(trayAvailable)
    , m_storage(std::move(storage))
    , m_profile(profile)
{
    setupUi();
    loadValues();
    refreshStorageSizes();
}

void SettingsDialog::setupUi()
{
    setWindowTitle(tr("Settings"));
    setModal(false);

    m_tabs = new QTabWidget(this);
    m_tabs->addTab(wrapInScroll(buildGeneralTab()), tr("General"));
    m_tabs->addTab(wrapInScroll(buildAppearanceTab()), tr("Appearance"));
    m_tabs->addTab(wrapInScroll(buildNotificationsTab()), tr("Notifications"));
    m_tabs->addTab(wrapInScroll(buildPrivacyTab()), tr("Privacy && Advanced"));

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_tabs, 1);
    layout->addWidget(buttons);
    // Tall tabs (Privacy & Advanced especially) scroll inside a fixed dialog
    // instead of stretching it past the screen.
    resize(580, 560);
}

QWidget* SettingsDialog::wrapInScroll(QWidget* content)
{
    auto* area = new QScrollArea(this);
    area->setWidget(content);
    area->setWidgetResizable(true);
    area->setFrameShape(QFrame::NoFrame);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->viewport()->setAutoFillBackground(false);
    content->setAutoFillBackground(false);
    return area;
}

QWidget* SettingsDialog::buildGeneralTab()
{
    auto* page = new QWidget(this);
    auto* outer = new QVBoxLayout(page);

    auto* windowBox = new QGroupBox(tr("Window"), page);
    auto* form = new QFormLayout(windowBox);
    m_closeAction = new QComboBox(windowBox);
    m_closeAction->addItem(tr("Minimize to tray"), static_cast<int>(CloseAction::MinimizeToTray));
    m_closeAction->addItem(tr("Quit"), static_cast<int>(CloseAction::Quit));
    connect(m_closeAction, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_settings.setCloseAction(static_cast<CloseAction>(m_closeAction->itemData(index).toInt()));
    });
    form->addRow(tr("When closing the window:"), m_closeAction);
    m_startMinimized = new QCheckBox(tr("Start hidden in the system tray"), windowBox);
    connect(m_startMinimized, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setStartMinimized(on); });
    form->addRow(QString(), m_startMinimized);
    m_trayLeftClick = new QCheckBox(tr("Left-clicking the tray icon shows/hides the window"), windowBox);
    connect(m_trayLeftClick, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setTrayLeftClickToggles(on); });
    form->addRow(QString(), m_trayLeftClick);
    m_autostart = new QCheckBox(tr("Start Whatsie automatically when I log in"), windowBox);
    connect(m_autostart, &QCheckBox::toggled, this, [this](bool on) { m_settings.setAutostart(on); });
    form->addRow(QString(), m_autostart);
    if (!m_trayAvailable) {
        auto* note =
            new QLabel(tr("No system tray was detected: the window will never be hidden."), windowBox);
        note->setWordWrap(true);
        note->setStyleSheet(u"color: palette(placeholder-text);"_s);
        form->addRow(QString(), note);
        m_closeAction->setEnabled(false);
        m_startMinimized->setEnabled(false);
        m_trayLeftClick->setEnabled(false);
    }
    outer->addWidget(windowBox);

    // Tray appearance preferences apply to the window/taskbar icon too, so they
    // stay usable even when no system tray is present (e.g. to un-hide it).
    auto* trayBox = new QGroupBox(tr("System tray"), page);
    auto* trayForm = new QFormLayout(trayBox);
    m_traySymbolic = new QCheckBox(tr("Use a monochrome (symbolic) icon"), trayBox);
    connect(m_traySymbolic, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setTraySymbolicIcon(on); });
    trayForm->addRow(QString(), m_traySymbolic);
    m_trayHidden = new QCheckBox(tr("Hide the tray icon"), trayBox);
    connect(m_trayHidden, &QCheckBox::toggled, this, [this](bool on) { m_settings.setTrayHidden(on); });
    trayForm->addRow(QString(), m_trayHidden);
    m_trayDim = new QCheckBox(tr("Dim the icon while disconnected"), trayBox);
    connect(m_trayDim, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setTrayDimWhenDisconnected(on); });
    trayForm->addRow(QString(), m_trayDim);
    outer->addWidget(trayBox);

    auto* downloadsBox = new QGroupBox(tr("Downloads"), page);
    auto* dform = new QFormLayout(downloadsBox);
    m_downloadDir = new QLineEdit(downloadsBox);
    m_downloadDir->setReadOnly(true);
    auto* browse = new QPushButton(tr("Change…"), downloadsBox);
    connect(browse, &QPushButton::clicked, this, [this] {
        const QString dir =
            QFileDialog::getExistingDirectory(this, tr("Download folder"), m_settings.downloadDirectory());
        if (!dir.isEmpty()) {
            m_settings.setDownloadDirectory(dir);
            m_downloadDir->setText(m_settings.downloadDirectory());
        }
    });
    auto* dirRow = new QHBoxLayout;
    dirRow->addWidget(m_downloadDir, 1);
    dirRow->addWidget(browse);
    dform->addRow(tr("Save files to:"), dirRow);
    m_askWhereToSave = new QCheckBox(tr("Always ask where to save"), downloadsBox);
    connect(m_askWhereToSave, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setAskWhereToSave(on); });
    dform->addRow(QString(), m_askWhereToSave);
    m_showDownloads = new QCheckBox(tr("Show the Downloads window when a download starts"), downloadsBox);
    connect(m_showDownloads, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setShowDownloadsOnStart(on); });
    dform->addRow(QString(), m_showDownloads);
    outer->addWidget(downloadsBox);
    outer->addStretch();
    return page;
}

QWidget* SettingsDialog::buildAppearanceTab()
{
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    m_theme = new QComboBox(page);
    m_theme->addItem(tr("Follow system"), static_cast<int>(Theme::System));
    m_theme->addItem(tr("Light"), static_cast<int>(Theme::Light));
    m_theme->addItem(tr("Dark"), static_cast<int>(Theme::Dark));
    connect(m_theme, &QComboBox::currentIndexChanged, this,
            [this](int index) { m_settings.setTheme(static_cast<Theme>(m_theme->itemData(index).toInt())); });
    form->addRow(tr("Theme:"), m_theme);

    auto makeZoom = [page] {
        auto* spin = new QDoubleSpinBox(page);
        spin->setRange(core::kMinZoom, core::kMaxZoom);
        spin->setSingleStep(core::kZoomStep);
        spin->setDecimals(2);
        spin->setSuffix(u"×"_s);
        return spin;
    };
    m_zoom = makeZoom();
    connect(m_zoom, &QDoubleSpinBox::valueChanged, this, [this](double v) { m_settings.setZoomFactor(v); });
    connect(&m_settings, &core::Settings::zoomFactorChanged, m_zoom, &QDoubleSpinBox::setValue);
    form->addRow(tr("Zoom (normal window):"), m_zoom);

    m_zoomMaximized = makeZoom();
    connect(m_zoomMaximized, &QDoubleSpinBox::valueChanged, this,
            [this](double v) { m_settings.setZoomFactorMaximized(v); });
    connect(&m_settings, &core::Settings::zoomFactorMaximizedChanged, m_zoomMaximized,
            &QDoubleSpinBox::setValue);
    form->addRow(tr("Zoom (maximized / full screen):"), m_zoomMaximized);

    m_smoothScrolling = new QCheckBox(tr("Smooth scrolling"), page);
    connect(m_smoothScrolling, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setSmoothScrolling(on); });
    form->addRow(QString(), m_smoothScrolling);

    m_interfaceScale = new QDoubleSpinBox(page);
    m_interfaceScale->setRange(core::Settings::kMinInterfaceScale, core::Settings::kMaxInterfaceScale);
    m_interfaceScale->setSingleStep(0.25);
    m_interfaceScale->setDecimals(2);
    m_interfaceScale->setSuffix(u"×"_s);
    m_interfaceScale->setToolTip(tr("Scales the whole interface. Applies after restarting Whatsie."));
    connect(m_interfaceScale, &QDoubleSpinBox::valueChanged, this,
            [this](double v) { m_settings.setInterfaceScale(v); });
    form->addRow(tr("Interface scale:"), m_interfaceScale);

    m_messageBlur = new QComboBox(page);
    m_messageBlur->addItem(tr("Off"), 0);
    m_messageBlur->addItem(tr("Light"), 1);
    m_messageBlur->addItem(tr("Medium"), 2);
    m_messageBlur->addItem(tr("Strong"), 3);
    connect(m_messageBlur, &QComboBox::currentIndexChanged, this,
            [this](int index) { m_settings.setMessageBlurLevel(m_messageBlur->itemData(index).toInt()); });
    connect(&m_settings, &core::Settings::messageBlurLevelChanged, this,
            [this](int level) { m_messageBlur->setCurrentIndex(m_messageBlur->findData(level)); });
    form->addRow(tr("Blur messages until hover:"), m_messageBlur);
    return page;
}

QWidget* SettingsDialog::buildNotificationsTab()
{
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    m_notificationsEnabled = new QCheckBox(tr("Show message notifications"), page);
    connect(m_notificationsEnabled, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setNotificationsEnabled(on); });
    form->addRow(QString(), m_notificationsEnabled);

    m_notificationSound = new QCheckBox(tr("Play the desktop's message sound"), page);
    connect(m_notificationSound, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setNotificationSound(on); });
    form->addRow(QString(), m_notificationSound);

    m_notificationTimeout = new QSpinBox(page);
    m_notificationTimeout->setRange(0, 120);
    m_notificationTimeout->setSpecialValueText(tr("Desktop default"));
    m_notificationTimeout->setSuffix(tr(" s"));
    connect(m_notificationTimeout, &QSpinBox::valueChanged, this,
            [this](int v) { m_settings.setNotificationTimeoutSec(v); });
    form->addRow(tr("Hide after:"), m_notificationTimeout);

    auto* test = new QPushButton(tr("Send test notification"), page);
    connect(test, &QPushButton::clicked, this, &SettingsDialog::testNotificationRequested);
    form->addRow(QString(), test);

    auto* note = new QLabel(tr("Temporary Do-not-disturb is in the tray icon menu."), page);
    note->setStyleSheet(u"color: palette(placeholder-text);"_s);
    note->setWordWrap(true);
    form->addRow(QString(), note);

    connect(m_notificationsEnabled, &QCheckBox::toggled, m_notificationSound, &QWidget::setEnabled);
    connect(m_notificationsEnabled, &QCheckBox::toggled, m_notificationTimeout, &QWidget::setEnabled);
    return page;
}

QWidget* SettingsDialog::buildPrivacyTab()
{
    auto* page = new QWidget(this);
    auto* outer = new QVBoxLayout(page);

    auto* permBox = new QGroupBox(tr("What WhatsApp may use"), page);
    auto* pl = new QVBoxLayout(permBox);
    auto* permNote = new QLabel(tr("Camera and microphone are on by default so calls work without a "
                                   "prompt. Turn anything off here."),
                                permBox);
    permNote->setWordWrap(true);
    permNote->setStyleSheet(u"color: palette(placeholder-text);"_s);
    pl->addWidget(permNote);
    pl->addWidget(new PermissionList(m_profile, QUrl(u"https://web.whatsapp.com"_s), permBox));
    outer->addWidget(permBox);

    auto* storageBox = new QGroupBox(tr("Storage"), page);
    auto* sform = new QFormLayout(storageBox);
    m_cacheSize = new QLabel(tr("calculating…"), storageBox);
    m_sessionSize = new QLabel(tr("calculating…"), storageBox);
    sform->addRow(tr("Cache:"), m_cacheSize);
    sform->addRow(tr("Session data:"), m_sessionSize);
    auto* clearCache = new QPushButton(tr("Clear cache"), storageBox);
    clearCache->setToolTip(tr("Removes cached web content. You stay logged in."));
    connect(clearCache, &QPushButton::clicked, this, [this] {
        Q_EMIT clearCacheRequested();
        refreshStorageSizes();
    });
    auto* clearSession = new QPushButton(tr("Log out && clear session…"), storageBox);
    clearSession->setToolTip(
        tr("Removes the WhatsApp session. Whatsie quits and you link your phone again."));
    connect(clearSession, &QPushButton::clicked, this, &SettingsDialog::clearSessionRequested);
    auto* buttons = new QHBoxLayout;
    buttons->addWidget(clearCache);
    buttons->addWidget(clearSession);
    buttons->addStretch();
    sform->addRow(QString(), buttons);
    outer->addWidget(storageBox);

    auto* spellBox = new QGroupBox(tr("Spell check"), page);
    auto* spellLayout = new QVBoxLayout(spellBox);
    m_spellCheck = new QCheckBox(tr("Check spelling as I type"), spellBox);
    connect(m_spellCheck, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setSpellCheckEnabled(on); });
    spellLayout->addWidget(m_spellCheck);

    // Let the user pick any bundled dictionary (they cannot install more yet —
    // on-demand downloads are a future feature — so we only ever offer what is
    // actually present, and say so honestly).
    const QStringList available =
        core::availableDictionaries(qEnvironmentVariable("QTWEBENGINE_DICTIONARIES_PATH"));
    if (available.isEmpty()) {
        auto* none = new QLabel(tr("No dictionaries are bundled, so spell check is inactive."), spellBox);
        none->setWordWrap(true);
        none->setStyleSheet(u"color: palette(placeholder-text);"_s);
        spellLayout->addWidget(none);
    } else {
        auto* row = new QFormLayout;
        m_spellLanguage = new QComboBox(spellBox);
        for (const QString& code : available) {
            const QLocale loc(code);
            const QString label = loc.language() == QLocale::AnyLanguage
                                      ? code
                                      : tr("%1 (%2) — %3")
                                            .arg(QLocale::languageToString(loc.language()),
                                                 QLocale::territoryToString(loc.territory()), code);
            m_spellLanguage->addItem(label, code);
        }
        const QString effective =
            core::resolveDictionary(m_settings.spellCheckLanguages().value(0), available);
        m_spellLanguage->setCurrentIndex(std::max(0, m_spellLanguage->findData(effective)));
        connect(m_spellLanguage, &QComboBox::currentIndexChanged, this, [this](int index) {
            m_settings.setSpellCheckLanguages({m_spellLanguage->itemData(index).toString()});
        });
        row->addRow(tr("Language:"), m_spellLanguage);
        spellLayout->addLayout(row);
    }
    outer->addWidget(spellBox);

    outer->addWidget(buildLockGroup());
    outer->addWidget(buildNetworkGroup());

    // Notes and buttons live in the QVBoxLayout, not in the QFormLayout: a
    // word-wrapped label placed in a form's field column mis-sizes and overlaps
    // the next row.
    auto* advBox = new QGroupBox(tr("Advanced"), page);
    auto* av = new QVBoxLayout(advBox);
    auto* aform = new QFormLayout;
    m_hardwareAcceleration = new QComboBox(advBox);
    m_hardwareAcceleration->addItem(tr("Automatic"), static_cast<int>(HardwareAcceleration::Auto));
    m_hardwareAcceleration->addItem(tr("Always on (ignore GPU blocklist)"),
                                    static_cast<int>(HardwareAcceleration::On));
    m_hardwareAcceleration->addItem(tr("Off"), static_cast<int>(HardwareAcceleration::Off));
    connect(m_hardwareAcceleration, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_settings.setHardwareAcceleration(
            static_cast<HardwareAcceleration>(m_hardwareAcceleration->itemData(index).toInt()));
    });
    aform->addRow(tr("Hardware acceleration:"), m_hardwareAcceleration);
    m_webrtcPublicOnly = new QCheckBox(tr("Route WebRTC through public interfaces only"), advBox);
    m_webrtcPublicOnly->setToolTip(tr("Hides local network addresses from calls. May break calls "
                                      "on some networks. Takes effect after a reload."));
    connect(m_webrtcPublicOnly, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setWebrtcPublicInterfacesOnly(on); });
    aform->addRow(QString(), m_webrtcPublicOnly);
    av->addLayout(aform);

    auto* hwNote =
        new QLabel(tr("Takes effect after restarting Whatsie. Turn off if the window stays blank."), advBox);
    hwNote->setWordWrap(true);
    hwNote->setStyleSheet(u"color: palette(placeholder-text);"_s);
    av->addWidget(hwNote);

    const QString logPath = core::LogSink::logFilePath();
    auto* logLabel = new QLabel(
        tr("Log file: %1").arg(logPath.isEmpty() ? tr("disabled (--no-log-file)") : logPath), advBox);
    logLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    logLabel->setWordWrap(true);
    av->addWidget(logLabel);
    auto* openLog = new QPushButton(QIcon::fromTheme(u"folder-open"_s), tr("Open log folder"), advBox);
    openLog->setEnabled(!logPath.isEmpty());
    connect(openLog, &QPushButton::clicked, this,
            [logPath] { platform::openDirectory(QFileInfo(logPath).absolutePath()); });
    av->addWidget(openLog, 0, Qt::AlignLeft);
    outer->addWidget(advBox);
    outer->addStretch();
    return page;
}

QWidget* SettingsDialog::buildLockGroup()
{
    auto* box = new QGroupBox(tr("Screen lock"), this);
    auto* outer = new QVBoxLayout(box);

    m_lockStatus = new QLabel(box);
    m_lockStatus->setWordWrap(true);
    outer->addWidget(m_lockStatus);

    auto* buttons = new QHBoxLayout;
    m_lockSet = new QPushButton(box);
    connect(m_lockSet, &QPushButton::clicked, this, [this] {
        PasscodeDialog dialog(m_settings.passcodeRecord(), this);
        if (dialog.exec() == QDialog::Accepted) {
            m_settings.setPasscode(core::makePasscode(dialog.newPasscode()));
            updateLockUi();
        }
    });
    m_lockRemove = new QPushButton(tr("Remove…"), box);
    connect(m_lockRemove, &QPushButton::clicked, this, [this] {
        bool ok = false;
        const QString current =
            QInputDialog::getText(this, tr("Remove passcode"), tr("Enter the current passcode:"),
                                  QLineEdit::Password, QString(), &ok);
        if (!ok) {
            return;
        }
        if (!core::verifyPasscode(current, m_settings.passcodeRecord())) {
            QMessageBox::warning(this, tr("Remove passcode"), tr("That passcode is incorrect."));
            return;
        }
        m_settings.clearPasscode();
        updateLockUi();
    });
    buttons->addWidget(m_lockSet);
    buttons->addWidget(m_lockRemove);
    buttons->addStretch();
    outer->addLayout(buttons);

    auto* triggers = new QFormLayout;
    m_lockOnStart = new QCheckBox(tr("Lock on startup"), box);
    connect(m_lockOnStart, &QCheckBox::toggled, this, [this](bool on) { m_settings.setLockOnStart(on); });
    triggers->addRow(QString(), m_lockOnStart);
    m_lockOnHide = new QCheckBox(tr("Lock when minimized to the tray"), box);
    connect(m_lockOnHide, &QCheckBox::toggled, this, [this](bool on) { m_settings.setLockOnHide(on); });
    triggers->addRow(QString(), m_lockOnHide);
    m_lockIdle = new QSpinBox(box);
    m_lockIdle->setRange(0, 120);
    m_lockIdle->setSpecialValueText(tr("Never"));
    m_lockIdle->setSuffix(tr(" min"));
    connect(m_lockIdle, &QSpinBox::valueChanged, this, [this](int v) { m_settings.setLockIdleMinutes(v); });
    triggers->addRow(tr("Lock after idle:"), m_lockIdle);
    outer->addLayout(triggers);

    return box;
}

void SettingsDialog::updateLockUi()
{
    const bool hasPasscode = m_settings.hasPasscode();
    m_lockStatus->setText(hasPasscode ? tr("A passcode is set. The app can be locked below.")
                                      : tr("No passcode set. Set one to enable locking."));
    m_lockSet->setText(hasPasscode ? tr("Change passcode…") : tr("Set passcode…"));
    m_lockRemove->setEnabled(hasPasscode);
    for (QWidget* w : QList<QWidget*>{m_lockOnStart, m_lockOnHide, m_lockIdle}) {
        w->setEnabled(hasPasscode);
    }
}

QWidget* SettingsDialog::buildNetworkGroup()
{
    auto* box = new QGroupBox(tr("Network proxy"), this);
    auto* outer = new QVBoxLayout(box);
    auto* form = new QFormLayout;

    m_proxyMode = new QComboBox(box);
    m_proxyMode->addItem(tr("Use system settings"), static_cast<int>(ProxyMode::System));
    m_proxyMode->addItem(tr("No proxy (direct)"), static_cast<int>(ProxyMode::None));
    m_proxyMode->addItem(tr("Manual"), static_cast<int>(ProxyMode::Manual));
    connect(m_proxyMode, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_settings.setProxyMode(static_cast<ProxyMode>(m_proxyMode->itemData(index).toInt()));
        updateProxyEnabled();
    });
    form->addRow(tr("Proxy:"), m_proxyMode);

    m_proxyType = new QComboBox(box);
    m_proxyType->addItem(tr("HTTP"), static_cast<int>(ProxyType::Http));
    m_proxyType->addItem(tr("SOCKS5"), static_cast<int>(ProxyType::Socks5));
    connect(m_proxyType, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_settings.setProxyType(static_cast<ProxyType>(m_proxyType->itemData(index).toInt()));
    });
    form->addRow(tr("Type:"), m_proxyType);

    auto* hostRow = new QHBoxLayout;
    m_proxyHost = new QLineEdit(box);
    m_proxyHost->setPlaceholderText(tr("host or IP"));
    connect(m_proxyHost, &QLineEdit::editingFinished, this,
            [this] { m_settings.setProxyHost(m_proxyHost->text().trimmed()); });
    m_proxyPort = new QSpinBox(box);
    m_proxyPort->setRange(1, 65535);
    connect(m_proxyPort, &QSpinBox::valueChanged, this, [this](int v) { m_settings.setProxyPort(v); });
    hostRow->addWidget(m_proxyHost, 1);
    hostRow->addWidget(new QLabel(tr("Port:"), box));
    hostRow->addWidget(m_proxyPort);
    form->addRow(tr("Server:"), hostRow);

    m_proxyUser = new QLineEdit(box);
    m_proxyUser->setPlaceholderText(tr("optional"));
    connect(m_proxyUser, &QLineEdit::editingFinished, this,
            [this] { m_settings.setProxyUser(m_proxyUser->text()); });
    form->addRow(tr("Username:"), m_proxyUser);

    m_proxyPassword = new QLineEdit(box);
    m_proxyPassword->setEchoMode(QLineEdit::Password);
    m_proxyPassword->setPlaceholderText(tr("kept for this session only"));
    connect(m_proxyPassword, &QLineEdit::textEdited, this,
            [this](const QString& text) { m_settings.setProxyPassword(text); });
    form->addRow(tr("Password:"), m_proxyPassword);

    outer->addLayout(form);
    auto* note = new QLabel(tr("Reload WhatsApp (F5) for proxy changes to take effect. The password "
                               "is never saved to disk."),
                            box);
    note->setWordWrap(true);
    note->setStyleSheet(u"color: palette(placeholder-text);"_s);
    outer->addWidget(note);
    return box;
}

void SettingsDialog::updateProxyEnabled()
{
    const bool manual = m_settings.proxyMode() == ProxyMode::Manual;
    const QList<QWidget*> fields{m_proxyType, m_proxyHost, m_proxyPort, m_proxyUser, m_proxyPassword};
    for (QWidget* w : fields) {
        w->setEnabled(manual);
    }
}

void SettingsDialog::refreshStorageSizes()
{
    for (const auto& [label, path] :
         {std::pair{m_cacheSize, m_storage.cacheDir}, std::pair{m_sessionSize, m_storage.sessionDir}}) {
        auto* watcher = new QFutureWatcher<qint64>(this);
        connect(watcher, &QFutureWatcher<qint64>::finished, this, [watcher, label] {
            label->setText(core::humanSize(watcher->result()));
            watcher->deleteLater();
        });
        watcher->setFuture(QtConcurrent::run([path] { return core::directorySize(path); }));
    }
}

void SettingsDialog::loadValues()
{
    m_closeAction->setCurrentIndex(m_closeAction->findData(static_cast<int>(m_settings.closeAction())));
    m_startMinimized->setChecked(m_settings.startMinimized());
    m_trayLeftClick->setChecked(m_settings.trayLeftClickToggles());
    m_autostart->setChecked(m_settings.autostart());
    m_traySymbolic->setChecked(m_settings.traySymbolicIcon());
    m_trayHidden->setChecked(m_settings.trayHidden());
    m_trayDim->setChecked(m_settings.trayDimWhenDisconnected());
    m_downloadDir->setText(m_settings.downloadDirectory());
    m_askWhereToSave->setChecked(m_settings.askWhereToSave());
    m_showDownloads->setChecked(m_settings.showDownloadsOnStart());
    m_theme->setCurrentIndex(m_theme->findData(static_cast<int>(m_settings.theme())));
    m_zoom->setValue(m_settings.zoomFactor());
    m_zoomMaximized->setValue(m_settings.zoomFactorMaximized());
    m_smoothScrolling->setChecked(m_settings.smoothScrolling());
    m_interfaceScale->setValue(m_settings.interfaceScale());
    m_messageBlur->setCurrentIndex(m_messageBlur->findData(m_settings.messageBlurLevel()));
    m_notificationsEnabled->setChecked(m_settings.notificationsEnabled());
    m_notificationSound->setChecked(m_settings.notificationSound());
    m_notificationSound->setEnabled(m_settings.notificationsEnabled());
    m_notificationTimeout->setValue(m_settings.notificationTimeoutSec());
    m_notificationTimeout->setEnabled(m_settings.notificationsEnabled());
    m_hardwareAcceleration->setCurrentIndex(
        m_hardwareAcceleration->findData(static_cast<int>(m_settings.hardwareAcceleration())));
    m_spellCheck->setChecked(m_settings.spellCheckEnabled());
    m_lockOnStart->setChecked(m_settings.lockOnStart());
    m_lockOnHide->setChecked(m_settings.lockOnHide());
    m_lockIdle->setValue(m_settings.lockIdleMinutes());
    updateLockUi();
    m_webrtcPublicOnly->setChecked(m_settings.webrtcPublicInterfacesOnly());
    m_proxyMode->setCurrentIndex(m_proxyMode->findData(static_cast<int>(m_settings.proxyMode())));
    m_proxyType->setCurrentIndex(m_proxyType->findData(static_cast<int>(m_settings.proxyType())));
    m_proxyHost->setText(m_settings.proxyHost());
    m_proxyPort->setValue(m_settings.proxyPort());
    m_proxyUser->setText(m_settings.proxyUser());
    m_proxyPassword->setText(m_settings.proxyPassword());
    updateProxyEnabled();
}

} // namespace whatsie::ui
