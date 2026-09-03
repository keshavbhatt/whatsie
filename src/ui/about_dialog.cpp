#include "ui/about_dialog.h"

#include "core/settings/settings.h"
#include "platform/file_manager.h"
#include "ui/bug_report_dialog.h"
#include "ui/diagnostics.h"

#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

namespace {
const QString kSourceUrl = u"https://github.com/keshavbhatt/whatsie"_s;
const QString kDonateUrl = u"https://www.paypal.com/paypalme/keshavnrj/11"_s;
const QString kRateUrl = u"snap://whatsie"_s;
const QString kMoreAppsUrl = u"https://ktechpit.com/USS/public/products.php"_s;
const QString kWebsiteUrl = u"https://ktechpit.com"_s;
} // namespace

AboutDialog::AboutDialog(const core::Settings& settings, const QString& userAgent, QWidget* parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_userAgent(userAgent)
{
    setupUi();
}

void AboutDialog::setupUi()
{
    setWindowTitle(tr("%1 | About").arg(QApplication::applicationDisplayName()));
    setModal(true);

    m_content = new QWidget(this);

    auto* icon = new QLabel(m_content);
    icon->setPixmap(QIcon(u":/icons/whatsie.svg"_s).pixmap(96, 96));
    icon->setFixedSize(96, 96);

    auto* name = new QLabel(u"<span style=\"font-size:17pt; font-weight:600;\">%1</span>"_s.arg(
                                QApplication::applicationDisplayName()),
                            m_content);
    auto* description = new QLabel(tr("WhatsApp Web client for Linux Desktop"), m_content);
    description->setWordWrap(true);
    auto* version =
        new QLabel(tr("Version %1").arg(QApplication::applicationVersion()), m_content);
    version->setStyleSheet(u"color: palette(placeholder-text);"_s);

    auto* author =
        new QLabel(tr("<b>Designed &amp; Developed by:</b> Keshav Bhatt<br>"
                      "<b>Website:</b> <a href=\"%1\">ktechpit.com</a>")
                       .arg(kWebsiteUrl),
                   m_content);
    author->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    author->setTextFormat(Qt::RichText);
    author->setOpenExternalLinks(false);
    author->setTextInteractionFlags(Qt::TextBrowserInteraction);
    connect(author, &QLabel::linkActivated, this,
            [](const QString& link) { platform::openUrl(link); });

    auto* identity = new QVBoxLayout;
    identity->setSpacing(2);
    identity->addStretch();
    identity->addWidget(name);
    identity->addWidget(description);
    identity->addWidget(version);
    identity->addSpacing(10);
    identity->addWidget(author);
    identity->addStretch();

    auto* top = new QHBoxLayout;
    top->setSpacing(16);
    top->addWidget(icon, 0, Qt::AlignTop);
    top->addLayout(identity, 1);

    const auto linkButton = [this](const QString& label, const QString& url) {
        auto* button = new QPushButton(label, m_content);
        connect(button, &QPushButton::clicked, this, [url] { platform::openUrl(url); });
        return button;
    };
    auto* actions = new QHBoxLayout;
    actions->setSpacing(8);
    actions->addWidget(linkButton(tr("Donate PayPal"), kDonateUrl));
    actions->addWidget(linkButton(tr("Rate in Store"), kRateUrl));
    actions->addWidget(linkButton(tr("More Applications"), kMoreAppsUrl));
    actions->addWidget(linkButton(tr("Source Code"), kSourceUrl));

    auto* separator = new QFrame(m_content);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    m_debugToggle = new QPushButton(tr("Show Debug Info"), m_content);
    m_debugToggle->setCheckable(true);
    m_debugToggle->setFlat(true);
    m_debugToggle->setStyleSheet(u"text-align:left;"_s);
    connect(m_debugToggle, &QPushButton::clicked, this, &AboutDialog::toggleDebugInfo);
    auto* copy = new QPushButton(tr("Copy"), m_content);
    copy->setToolTip(
        tr("Copies versions, paths and recent log lines for a bug report. No messages are included."));
    connect(copy, &QPushButton::clicked, this, &AboutDialog::copyDiagnostics);
    auto* report = new QPushButton(tr("Report a bug…"), m_content);
    connect(report, &QPushButton::clicked, this, &AboutDialog::reportBug);

    auto* debugRow = new QHBoxLayout;
    debugRow->addWidget(m_debugToggle);
    debugRow->addStretch();
    debugRow->addWidget(copy);
    debugRow->addWidget(report);

    m_debugText = new QPlainTextEdit(m_content);
    m_debugText->setReadOnly(true);
    m_debugText->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_debugText->setPlainText(buildDiagnostics(m_settings, m_userAgent));
    m_debugText->setMinimumHeight(180);
    m_debugText->hide();

    auto* content = new QVBoxLayout(m_content);
    content->setContentsMargins(20, 20, 20, 16);
    content->setSpacing(10);
    content->addLayout(top);
    content->addLayout(actions);
    content->addWidget(separator);
    content->addLayout(debugRow);
    content->addWidget(m_debugText);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_content);
    setMinimumWidth(540);
}

void AboutDialog::toggleDebugInfo()
{
    const bool show = !m_debugText->isVisible();
    m_debugText->setVisible(show);
    m_debugToggle->setText(show ? tr("Hide Debug Info") : tr("Show Debug Info"));
    // Force the layouts to release/claim the panel's space now, then fit the
    // window height to the result — otherwise hiding leaves the dialog at its
    // taller size with an empty gap.
    if (m_content->layout() != nullptr) {
        m_content->layout()->activate();
    }
    if (layout() != nullptr) {
        layout()->activate();
    }
    resize(width(), sizeHint().height());
}

void AboutDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    // The opacity effect is attached only while animating and dropped after: a
    // QGraphicsEffect left on a widget forces offscreen rendering that can paint
    // it black when another dialog later re-exposes it.
    if (m_content == nullptr || m_content->graphicsEffect() != nullptr) {
        return; // already animated once
    }
    auto* effect = new QGraphicsOpacityEffect(m_content);
    m_content->setGraphicsEffect(effect);
    effect->setOpacity(0.0);
    auto* fade = new QPropertyAnimation(effect, "opacity", this);
    fade->setDuration(320);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->setEasingCurve(QEasingCurve::OutCubic);
    connect(fade, &QPropertyAnimation::finished, this, [this, effect] {
        if (m_content->graphicsEffect() == effect) {
            m_content->setGraphicsEffect(nullptr);
        }
    });
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

void AboutDialog::copyDiagnostics()
{
    QApplication::clipboard()->setText(buildDiagnostics(m_settings, m_userAgent));
}

void AboutDialog::reportBug()
{
    BugReportDialog dialog(m_settings, m_userAgent, this);
    dialog.exec();
}

} // namespace whatsie::ui
