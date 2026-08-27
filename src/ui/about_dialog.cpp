#include "ui/about_dialog.h"

#include "core/settings/settings.h"
#include "platform/platform_info.h"
#include "ui/diagnostics.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

namespace {
const QString kSourceUrl = u"https://github.com/keshavbhatt/whatsie"_s;
const QString kIssuesUrl = u"https://github.com/keshavbhatt/whatsie/issues"_s;
const QString kDonateUrl = u"https://paypal.me/keshavnrj/5"_s;
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
    setWindowTitle(tr("About Whatsie"));
    setModal(true);

    auto* icon = new QLabel(this);
    icon->setPixmap(QIcon(u":/icons/whatsie.svg"_s).pixmap(96, 96));
    icon->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    auto* title = new QLabel(u"<h2>Whatsie</h2>"_s, this);
    auto* version = new QLabel(tr("Version %1").arg(QApplication::applicationVersion()), this);
    auto* blurb = new QLabel(tr("WhatsApp Web desktop client built with Qt WebEngine."), this);
    blurb->setWordWrap(true);
    auto* host = new QLabel(platform::describeHost(), this);
    host->setWordWrap(true);
    host->setStyleSheet(u"color: palette(placeholder-text);"_s);
    auto* links = new QLabel(u"<a href=\"%1\">%2</a> · <a href=\"%3\">%4</a> · <a href=\"%5\">%6</a>"_s.arg(
                                 kSourceUrl, tr("Source code"), kIssuesUrl, tr("Report a bug"), kDonateUrl,
                                 tr("Support the project")),
                             this);
    links->setOpenExternalLinks(true);

    auto* text = new QVBoxLayout;
    text->addWidget(title);
    text->addWidget(version);
    text->addWidget(blurb);
    text->addWidget(host);
    text->addWidget(links);
    text->addStretch();

    auto* top = new QHBoxLayout;
    top->addWidget(icon);
    top->addSpacing(12);
    top->addLayout(text, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto* copy = buttons->addButton(tr("Copy diagnostics"), QDialogButtonBox::ActionRole);
    copy->setToolTip(
        tr("Copies versions, paths and recent log lines for a bug report. No messages are included."));
    connect(copy, &QPushButton::clicked, this, &AboutDialog::copyDiagnostics);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(top);
    layout->addWidget(buttons);
    setMinimumWidth(460);
}

void AboutDialog::copyDiagnostics()
{
    QApplication::clipboard()->setText(buildDiagnostics(m_settings, m_userAgent));
}

} // namespace whatsie::ui
