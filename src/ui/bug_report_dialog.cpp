#include "ui/bug_report_dialog.h"

#include "platform/crash_handler.h"
#include "platform/file_manager.h"
#include "ui/diagnostics.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

namespace {
constexpr int kMaxTitle = 80;
} // namespace

BugReportDialog::BugReportDialog(const core::Settings& settings, QString userAgent, QWidget* parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_userAgent(std::move(userAgent))
{
    setupUi();
}

void BugReportDialog::setupUi()
{
    setWindowTitle(tr("Report a bug"));
    setModal(true);

    auto* intro = new QLabel(
        tr("A pre-filled GitHub issue will open in your browser. The diagnostics are copied to your "
           "clipboard — paste them in at the end of the issue. No messages or chats are included."),
        this);
    intro->setWordWrap(true);

    auto* titleLabel = new QLabel(tr("Title"), this);
    m_title = new QLineEdit(this);
    m_title->setMaxLength(kMaxTitle);
    m_title->setPlaceholderText(tr("Short summary of the problem"));

    auto* descriptionLabel = new QLabel(tr("What happened?"), this);
    m_description = new QPlainTextEdit(this);
    m_description->setPlaceholderText(
        tr("e.g. The window turns black after I close a chat, and only a restart fixes it."));
    m_description->setMinimumHeight(100);

    m_includeCrash = new QCheckBox(tr("Include the last crash report"), this);
    const bool hasCrash = !platform::lastCrashReport().isEmpty();
    m_includeCrash->setChecked(hasCrash);
    m_includeCrash->setVisible(hasCrash);

    m_status = new QLabel(this);
    m_status->setWordWrap(true);
    m_status->setStyleSheet(u"color: palette(placeholder-text);"_s);

    auto* copy = new QPushButton(tr("Copy diagnostics"), this);
    auto* open = new QPushButton(tr("Open GitHub Issue"), this);
    open->setDefault(true);
    open->setProperty("whatsiePrimary", true);
    auto* close = new QPushButton(tr("Close"), this);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(copy);
    buttons->addStretch();
    buttons->addWidget(open);
    buttons->addWidget(close);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(intro);
    layout->addSpacing(6);
    layout->addWidget(titleLabel);
    layout->addWidget(m_title);
    layout->addWidget(descriptionLabel);
    layout->addWidget(m_description);
    layout->addWidget(m_includeCrash);
    layout->addWidget(m_status);
    layout->addLayout(buttons);
    setMinimumWidth(560);

    connect(copy, &QPushButton::clicked, this, &BugReportDialog::copyDiagnostics);
    connect(open, &QPushButton::clicked, this, &BugReportDialog::submit);
    connect(close, &QPushButton::clicked, this, &QDialog::reject);
}

QString BugReportDialog::diagnostics() const
{
    return buildDiagnostics(m_settings, m_userAgent, 200, m_includeCrash->isChecked());
}

void BugReportDialog::copyDiagnostics()
{
    QApplication::clipboard()->setText(diagnostics());
    m_status->setText(tr("Diagnostics copied to the clipboard."));
}

void BugReportDialog::submit()
{
    QApplication::clipboard()->setText(diagnostics());
    const QString url = bugReportUrl(m_userAgent, m_title->text(), m_description->toPlainText());
    if (platform::openUrl(url)) {
        m_status->setText(tr("A GitHub issue is opening in your browser. Paste your clipboard "
                             "(Ctrl+V) at the end of the issue to attach the diagnostics."));
    } else {
        // Don't leave the user staring at a dead button: the report is already on
        // the clipboard, so tell them and offer the URL to open by hand.
        m_status->setText(tr("Couldn't open the browser automatically. Your report is on the "
                             "clipboard — open this URL and paste it in:\n%1")
                              .arg(url));
        m_status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }
}

} // namespace whatsie::ui
