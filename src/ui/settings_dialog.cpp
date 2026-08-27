#include "ui/settings_dialog.h"

#include "core/downloads/file_naming.h"
#include "core/log_sink.h"
#include "core/settings/settings.h"
#include "core/storage_policy.h"
#include "core/zoom_policy.h"
#include "platform/file_manager.h"
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
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QtConcurrent>

using namespace Qt::StringLiterals;
using whatsie::core::CloseAction;
using whatsie::core::HardwareAcceleration;
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
    m_tabs->addTab(buildGeneralTab(), tr("General"));
    m_tabs->addTab(buildAppearanceTab(), tr("Appearance"));
    m_tabs->addTab(buildNotificationsTab(), tr("Notifications"));
    m_tabs->addTab(buildPrivacyTab(), tr("Privacy && Advanced"));

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_tabs, 1);
    layout->addWidget(buttons);
    resize(560, 460);
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
    m_downloadDir->setText(m_settings.downloadDirectory());
    m_askWhereToSave->setChecked(m_settings.askWhereToSave());
    m_showDownloads->setChecked(m_settings.showDownloadsOnStart());
    m_theme->setCurrentIndex(m_theme->findData(static_cast<int>(m_settings.theme())));
    m_zoom->setValue(m_settings.zoomFactor());
    m_zoomMaximized->setValue(m_settings.zoomFactorMaximized());
    m_smoothScrolling->setChecked(m_settings.smoothScrolling());
    m_messageBlur->setCurrentIndex(m_messageBlur->findData(m_settings.messageBlurLevel()));
    m_notificationsEnabled->setChecked(m_settings.notificationsEnabled());
    m_notificationSound->setChecked(m_settings.notificationSound());
    m_notificationSound->setEnabled(m_settings.notificationsEnabled());
    m_notificationTimeout->setValue(m_settings.notificationTimeoutSec());
    m_notificationTimeout->setEnabled(m_settings.notificationsEnabled());
    m_hardwareAcceleration->setCurrentIndex(
        m_hardwareAcceleration->findData(static_cast<int>(m_settings.hardwareAcceleration())));
}

} // namespace whatsie::ui
