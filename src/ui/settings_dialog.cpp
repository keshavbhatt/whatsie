#include "ui/settings_dialog.h"

#include "core/log_sink.h"
#include "core/settings/settings.h"
#include "core/zoom_policy.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QUrl>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;
using whatsie::core::CloseAction;
using whatsie::core::Theme;

namespace whatsie::ui {

SettingsDialog::SettingsDialog(core::Settings& settings, bool trayAvailable, QWidget* parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_trayAvailable(trayAvailable)
{
    setupUi();
    loadValues();
}

void SettingsDialog::setupUi()
{
    setWindowTitle(tr("Settings"));
    setModal(false);

    m_tabs = new QTabWidget(this);
    m_tabs->addTab(buildGeneralTab(), tr("General"));
    m_tabs->addTab(buildAppearanceTab(), tr("Appearance"));
    m_tabs->addTab(buildNotificationsTab(), tr("Notifications"));
    m_tabs->addTab(buildAdvancedTab(), tr("Advanced"));

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_tabs, 1);
    layout->addWidget(buttons);
    resize(520, 400);
}

QWidget* SettingsDialog::buildGeneralTab()
{
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    m_closeAction = new QComboBox(page);
    m_closeAction->addItem(tr("Minimize to tray"), static_cast<int>(CloseAction::MinimizeToTray));
    m_closeAction->addItem(tr("Quit"), static_cast<int>(CloseAction::Quit));
    connect(m_closeAction, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_settings.setCloseAction(static_cast<CloseAction>(m_closeAction->itemData(index).toInt()));
    });
    form->addRow(tr("When closing the window:"), m_closeAction);

    m_startMinimized = new QCheckBox(tr("Start hidden in the system tray"), page);
    connect(m_startMinimized, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setStartMinimized(on); });
    form->addRow(QString(), m_startMinimized);

    m_trayLeftClick = new QCheckBox(tr("Left-clicking the tray icon shows/hides the window"), page);
    connect(m_trayLeftClick, &QCheckBox::toggled, this,
            [this](bool on) { m_settings.setTrayLeftClickToggles(on); });
    form->addRow(QString(), m_trayLeftClick);

    if (!m_trayAvailable) {
        auto* note = new QLabel(tr("No system tray was detected: the window will never be hidden."), page);
        note->setWordWrap(true);
        note->setStyleSheet(u"color: palette(placeholder-text);"_s);
        form->addRow(QString(), note);
        m_closeAction->setEnabled(false);
        m_startMinimized->setEnabled(false);
        m_trayLeftClick->setEnabled(false);
    }
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

QWidget* SettingsDialog::buildAdvancedTab()
{
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    const QString logPath = core::LogSink::logFilePath();
    auto* logLabel = new QLabel(logPath.isEmpty() ? tr("disabled (--no-log-file)") : logPath, page);
    logLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    logLabel->setWordWrap(true);
    form->addRow(tr("Log file:"), logLabel);

    auto* openLog = new QPushButton(tr("Open log folder"), page);
    openLog->setEnabled(!logPath.isEmpty());
    connect(openLog, &QPushButton::clicked, this,
            [logPath] { QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(logPath).absolutePath())); });
    form->addRow(QString(), openLog);

    auto* settingsLabel = new QLabel(m_settings.fileName(), page);
    settingsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    settingsLabel->setWordWrap(true);
    form->addRow(tr("Settings file:"), settingsLabel);
    return page;
}

void SettingsDialog::loadValues()
{
    m_closeAction->setCurrentIndex(m_closeAction->findData(static_cast<int>(m_settings.closeAction())));
    m_startMinimized->setChecked(m_settings.startMinimized());
    m_trayLeftClick->setChecked(m_settings.trayLeftClickToggles());
    m_theme->setCurrentIndex(m_theme->findData(static_cast<int>(m_settings.theme())));
    m_zoom->setValue(m_settings.zoomFactor());
    m_zoomMaximized->setValue(m_settings.zoomFactorMaximized());
    m_smoothScrolling->setChecked(m_settings.smoothScrolling());
    m_notificationsEnabled->setChecked(m_settings.notificationsEnabled());
    m_notificationSound->setChecked(m_settings.notificationSound());
    m_notificationSound->setEnabled(m_settings.notificationsEnabled());
    m_notificationTimeout->setValue(m_settings.notificationTimeoutSec());
    m_notificationTimeout->setEnabled(m_settings.notificationsEnabled());
}

} // namespace whatsie::ui
