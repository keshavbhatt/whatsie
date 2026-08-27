#include "ui/lock_screen.h"

#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

LockScreen::LockScreen(QWidget* parent)
    : QWidget(parent)
{
    setAutoFillBackground(true); // opaque: never let the page show through

    auto* icon = new QLabel(this);
    icon->setPixmap(QIcon(u":/icons/whatsie.svg"_s).pixmap(72, 72));
    icon->setAlignment(Qt::AlignCenter);

    auto* title = new QLabel(tr("Whatsie is locked"), this);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPointSizeF(titleFont.pointSizeF() * 1.5);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_input = new QLineEdit(this);
    m_input->setEchoMode(QLineEdit::Password);
    m_input->setPlaceholderText(tr("Enter passcode"));
    m_input->setAlignment(Qt::AlignCenter);
    m_input->setMaxLength(128);
    m_input->setFixedWidth(240);
    connect(m_input, &QLineEdit::returnPressed, this, &LockScreen::submit);

    m_unlock = new QPushButton(tr("Unlock"), this);
    m_unlock->setProperty("whatsiePrimary", true);
    m_unlock->setFixedWidth(240);
    connect(m_unlock, &QPushButton::clicked, this, &LockScreen::submit);

    m_status = new QLabel(this);
    m_status->setAlignment(Qt::AlignCenter);
    m_status->setWordWrap(true);
    m_status->setStyleSheet(u"color: palette(placeholder-text);"_s);

    auto* column = new QVBoxLayout;
    column->setSpacing(14);
    column->addStretch();
    column->addWidget(icon);
    column->addWidget(title);
    column->addSpacing(6);
    column->addWidget(m_input, 0, Qt::AlignHCenter);
    column->addWidget(m_unlock, 0, Qt::AlignHCenter);
    column->addWidget(m_status);
    column->addStretch();

    auto* outer = new QVBoxLayout(this);
    outer->addLayout(column);
}

void LockScreen::submit()
{
    if (!m_input->text().isEmpty()) {
        Q_EMIT unlockRequested(m_input->text());
    }
}

void LockScreen::reset()
{
    m_input->clear();
    m_input->setEnabled(true);
    m_unlock->setEnabled(true);
    m_status->clear();
    m_input->setFocus();
}

void LockScreen::showError(const QString& message)
{
    m_input->clear();
    m_input->selectAll();
    m_status->setStyleSheet(u"color: #e5504a;"_s);
    m_status->setText(message);
    m_input->setFocus();
}

void LockScreen::showThrottle(int secondsRemaining)
{
    const bool throttled = secondsRemaining > 0;
    m_input->setEnabled(!throttled);
    m_unlock->setEnabled(!throttled);
    if (throttled) {
        m_status->setStyleSheet(u"color: #e5504a;"_s);
        m_status->setText(tr("Too many attempts. Try again in %n second(s).", nullptr, secondsRemaining));
    } else {
        reset();
    }
}

} // namespace whatsie::ui
