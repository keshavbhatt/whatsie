#include "ui/passcode_dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace {
constexpr int kMinPasscodeLength = 4;
}

namespace whatsie::ui {

PasscodeDialog::PasscodeDialog(const core::PasscodeRecord& existing, QWidget* parent)
    : QDialog(parent)
    , m_existing(existing)
{
    setWindowTitle(existing.isValid() ? tr("Change passcode") : tr("Set passcode"));
    setModal(true);

    auto* form = new QFormLayout;
    if (existing.isValid()) {
        m_current = new QLineEdit(this);
        m_current->setEchoMode(QLineEdit::Password);
        form->addRow(tr("Current passcode:"), m_current);
    }
    m_new = new QLineEdit(this);
    m_new->setEchoMode(QLineEdit::Password);
    form->addRow(tr("New passcode:"), m_new);
    m_confirm = new QLineEdit(this);
    m_confirm->setEchoMode(QLineEdit::Password);
    form->addRow(tr("Confirm:"), m_confirm);

    m_error = new QLabel(this);
    m_error->setWordWrap(true);
    m_error->setStyleSheet(u"color: #e5504a;"_s);
    m_error->hide();

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &PasscodeDialog::tryAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(m_error);
    layout->addWidget(buttons);
    setMinimumWidth(320);
}

void PasscodeDialog::tryAccept()
{
    auto fail = [this](const QString& message) {
        m_error->setText(message);
        m_error->show();
    };
    if (m_existing.isValid() && !core::verifyPasscode(m_current->text(), m_existing)) {
        fail(tr("The current passcode is incorrect."));
        return;
    }
    if (m_new->text().length() < kMinPasscodeLength) {
        fail(tr("Use at least %n character(s).", nullptr, kMinPasscodeLength));
        return;
    }
    if (m_new->text() != m_confirm->text()) {
        fail(tr("The new passcodes do not match."));
        return;
    }
    accept();
}

QString PasscodeDialog::newPasscode() const
{
    return m_new->text();
}

} // namespace whatsie::ui
