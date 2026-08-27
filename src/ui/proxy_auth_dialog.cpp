#include "ui/proxy_auth_dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

ProxyAuthDialog::ProxyAuthDialog(const QString& proxyHost, const QString& knownUser, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Proxy authentication"));
    setModal(true);

    auto* intro = new QLabel(tr("The proxy %1 requires a username and password.")
                                 .arg(proxyHost.isEmpty() ? tr("server") : proxyHost),
                             this);
    intro->setWordWrap(true);

    m_user = new QLineEdit(knownUser, this);
    m_password = new QLineEdit(this);
    m_password->setEchoMode(QLineEdit::Password);

    auto* form = new QFormLayout;
    form->addRow(tr("Username:"), m_user);
    form->addRow(tr("Password:"), m_password);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(intro);
    layout->addLayout(form);
    layout->addWidget(buttons);
    setMinimumWidth(340);

    (knownUser.isEmpty() ? m_user : m_password)->setFocus();
}

QString ProxyAuthDialog::user() const
{
    return m_user->text();
}

QString ProxyAuthDialog::password() const
{
    return m_password->text();
}

} // namespace whatsie::ui
