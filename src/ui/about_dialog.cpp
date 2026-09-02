#include "ui/about_dialog.h"

#include "core/settings/settings.h"
#include "platform/crash_handler.h"
#include "platform/file_manager.h"
#include "ui/diagnostics.h"

#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

namespace {
const QString kSourceUrl = u"https://github.com/keshavbhatt/whatsie"_s;
const QString kDonateUrl = u"https://paypal.me/keshavnrj/5"_s;
const QString kRateUrl = u"snap://whatsie"_s;
const QString kMoreAppsUrl = u"https://snapcraft.io/publisher/keshavnrj"_s;
const QString kWebsiteUrl = u"https://ktechpit.com"_s;
const QString kAuthorEmail = u"keshavnrj@gmail.com"_s;
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
    setWindowTitle(tr("%1 | About").arg(QApplication::applicationName()));
    setModal(true);

    m_content = new QWidget(this);

    auto* icon = new QLabel(m_content);
    icon->setPixmap(QIcon(u":/icons/whatsie.svg"_s).pixmap(120, 120));
    icon->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    auto* name = new QLabel(u"<span style=\"font-size:18pt;\">%1</span>"_s.arg(
                                QApplication::applicationName()),
                            m_content);
    auto* description = new QLabel(tr("WhatsApp Web client for Linux Desktop"), m_content);
    description->setWordWrap(true);
    auto* version =
        new QLabel(tr("Version: %1").arg(QApplication::applicationVersion()), m_content);
    version->setStyleSheet(u"color: palette(placeholder-text);"_s);

    auto* author =
        new QLabel(tr("<p><b>Designed &amp; Developed by:</b> Keshav Bhatt</p>"
                      "<p><b>Email:</b> <a href=\"mailto:%1\">%1</a></p>"
                      "<p><b>Website:</b> <a href=\"%2\">ktechpit.com</a></p>")
                       .arg(kAuthorEmail, kWebsiteUrl),
                   m_content);
    author->setOpenExternalLinks(false);
    author->setTextInteractionFlags(Qt::TextBrowserInteraction);
    connect(author, &QLabel::linkActivated, this,
            [](const QString& link) { platform::openUrl(link); });

    auto* identity = new QVBoxLayout;
    identity->setSpacing(4);
    identity->addWidget(name);
    identity->addWidget(description);
    identity->addWidget(version);
    identity->addSpacing(8);
    identity->addWidget(author);
    identity->addStretch();

    auto* top = new QHBoxLayout;
    top->addWidget(icon, 0, Qt::AlignTop);
    top->addSpacing(16);
    top->addLayout(identity, 1);

    const auto linkButton = [this](const QString& label, const QString& url) {
        auto* button = new QPushButton(label, m_content);
        connect(button, &QPushButton::clicked, this, [url] { platform::openUrl(url); });
        return button;
    };
    auto* actions = new QHBoxLayout;
    actions->addWidget(linkButton(tr("Donate PayPal"), kDonateUrl));
    actions->addWidget(linkButton(tr("Rate in Store"), kRateUrl));
    actions->addWidget(linkButton(tr("More Applications"), kMoreAppsUrl));
    actions->addWidget(linkButton(tr("Source Code"), kSourceUrl));

    m_debugToggle = new QPushButton(tr("Show Debug Info"), m_content);
    m_debugToggle->setCheckable(true);
    connect(m_debugToggle, &QPushButton::clicked, this, &AboutDialog::toggleDebugInfo);
    auto* copy = new QPushButton(tr("Copy"), m_content);
    copy->setToolTip(
        tr("Copies versions, paths and recent log lines for a bug report. No messages are included."));
    connect(copy, &QPushButton::clicked, this, &AboutDialog::copyDiagnostics);
    auto* report = new QPushButton(tr("Report a bug…"), m_content);
    report->setToolTip(tr("Opens a pre-filled GitHub issue with diagnostics (and the last crash, if any)."));
    connect(report, &QPushButton::clicked, this, &AboutDialog::reportBug);

    auto* debugRow = new QHBoxLayout;
    debugRow->addWidget(m_debugToggle);
    debugRow->addWidget(copy);
    debugRow->addWidget(report);
    debugRow->addStretch();

    m_debugText = new QPlainTextEdit(m_content);
    m_debugText->setReadOnly(true);
    m_debugText->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_debugText->setPlainText(buildDiagnostics(m_settings, m_userAgent));
    m_debugText->setMinimumHeight(180);
    m_debugText->hide();

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, m_content);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* content = new QVBoxLayout(m_content);
    content->addLayout(top);
    content->addSpacing(8);
    content->addLayout(actions);
    content->addLayout(debugRow);
    content->addWidget(m_debugText);
    content->addWidget(buttons);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_content);
    setMinimumWidth(520);
}

void AboutDialog::toggleDebugInfo()
{
    const bool show = !m_debugText->isVisible();
    m_debugText->setVisible(show);
    m_debugToggle->setText(show ? tr("Hide Debug Info") : tr("Show Debug Info"));
    if (show) {
        adjustSize();
    } else {
        resize(width(), minimumSizeHint().height());
    }
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
    // Full diagnostics to the clipboard (the URL carries a trimmed copy), then a
    // pre-filled GitHub issue in the browser.
    QApplication::clipboard()->setText(buildDiagnostics(m_settings, m_userAgent));
    platform::openUrl(bugReportUrl(m_settings, m_userAgent));
    QMessageBox::information(
        this, tr("Report a bug"),
        tr("A new GitHub issue is opening in your browser, pre-filled with diagnostics"
           "%1.\n\nYour full diagnostics were also copied to the clipboard — paste them into the "
           "issue if the form looks short. No messages or chats are included.")
            .arg(platform::lastCrashReport().isEmpty() ? QString() : tr(" and the last crash")));
}

} // namespace whatsie::ui
