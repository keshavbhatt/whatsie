#include "ui/bug_report_dialog.h"

#include "platform/crash_handler.h"
#include "platform/file_manager.h"
#include "ui/diagnostics.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

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
        tr("Describe what went wrong. A pre-filled GitHub issue will open in your browser and the "
           "full report — including diagnostics — is copied to your clipboard to paste in."),
        this);
    intro->setWordWrap(true);

    auto* summaryLabel = new QLabel(tr("What happened?"), this);
    m_summary = new QPlainTextEdit(this);
    m_summary->setPlaceholderText(
        tr("e.g. The window turns black after I close a chat, and only a restart fixes it."));
    m_summary->setMinimumHeight(90);

    m_includeCrash = new QCheckBox(tr("Include the last crash report"), this);
    const bool hasCrash = !platform::lastCrashReport().isEmpty();
    m_includeCrash->setChecked(hasCrash);
    m_includeCrash->setVisible(hasCrash);

    m_detailsToggle = new QPushButton(tr("Show diagnostics that will be attached"), this);
    m_detailsToggle->setCheckable(true);
    m_detailsToggle->setFlat(true);
    m_detailsToggle->setStyleSheet(u"text-align:left;"_s);

    m_details = new QPlainTextEdit(this);
    m_details->setReadOnly(true);
    m_details->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_details->setMinimumHeight(160);
    m_details->hide();

    m_status = new QLabel(this);
    m_status->setWordWrap(true);
    m_status->setStyleSheet(u"color: palette(placeholder-text);"_s);

    auto* copy = new QPushButton(tr("Copy report"), this);
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
    layout->addWidget(summaryLabel);
    layout->addWidget(m_summary);
    layout->addWidget(m_includeCrash);
    layout->addWidget(m_detailsToggle);
    layout->addWidget(m_details);
    layout->addWidget(m_status);
    layout->addLayout(buttons);
    setMinimumWidth(560);

    connect(m_detailsToggle, &QPushButton::toggled, this, [this](bool on) {
        m_detailsToggle->setText(on ? tr("Hide diagnostics that will be attached")
                                    : tr("Show diagnostics that will be attached"));
        if (on) {
            refreshDetails();
        }
        m_details->setVisible(on);
        adjustSize();
    });
    connect(m_includeCrash, &QCheckBox::toggled, this, [this] {
        if (m_details->isVisible()) {
            refreshDetails();
        }
    });
    connect(copy, &QPushButton::clicked, this, &BugReportDialog::copyReport);
    connect(open, &QPushButton::clicked, this, &BugReportDialog::submit);
    connect(close, &QPushButton::clicked, this, &QDialog::reject);
}

void BugReportDialog::refreshDetails()
{
    m_details->setPlainText(bugReportBody(m_settings, m_userAgent, m_summary->toPlainText(),
                                          m_includeCrash->isChecked()));
}

void BugReportDialog::copyReport()
{
    QApplication::clipboard()->setText(bugReportBody(m_settings, m_userAgent,
                                                     m_summary->toPlainText(),
                                                     m_includeCrash->isChecked()));
    m_status->setText(tr("Full report copied to the clipboard."));
}

void BugReportDialog::submit()
{
    QApplication::clipboard()->setText(bugReportBody(m_settings, m_userAgent,
                                                     m_summary->toPlainText(),
                                                     m_includeCrash->isChecked()));
    platform::openUrl(bugReportUrl(m_userAgent, m_summary->toPlainText()));
    m_status->setText(tr("A GitHub issue is opening in your browser. Your full report is on the "
                         "clipboard — paste it (Ctrl+V) into the issue. No messages are included."));
}

} // namespace whatsie::ui
