// SPDX-FileCopyrightText: 2026 Keshav Bhatt (Ktechpit)
// SPDX-License-Identifier: MIT
#include "AppCard.h"

#include "AppLinks.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace ktechpit {

namespace {
constexpr int kIconSize = 40;

// The description in up to two lines, in the font it ends up with (a host's style sheet may set
// it after the card is built).
class TwoLineLabel : public QLabel
{
public:
    TwoLineLabel(const QString& text, QWidget* parent)
        : QLabel(text, parent)
    {
        setWordWrap(true);
        setAlignment(Qt::AlignTop | Qt::AlignLeft);
        fitTwoLines();
    }

protected:
    void changeEvent(QEvent* event) override
    {
        QLabel::changeEvent(event);
        if (event->type() == QEvent::FontChange) {
            fitTwoLines();
        }
    }

private:
    void fitTwoLines() { setMaximumHeight(fontMetrics().lineSpacing() * 2); }
};

// One line of text, shortened with an ellipsis when the card is narrow (the card's tooltip has
// it whole), for CatalogConfig::oneLineText.
class OneLineLabel : public QLabel
{
public:
    OneLineLabel(const QString& text, QWidget* parent)
        : QLabel(text, parent)
        , m_full(text)
    {
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        setMinimumWidth(0);
    }

    QSize sizeHint() const override { return {fontMetrics().horizontalAdvance(m_full), QLabel::sizeHint().height()}; }

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        QLabel::resizeEvent(event);
        elide();
    }
    void changeEvent(QEvent* event) override
    {
        QLabel::changeEvent(event);
        if (event->type() == QEvent::FontChange) {
            elide();
        }
    }

private:
    void elide() { setText(fontMetrics().elidedText(m_full, Qt::ElideRight, width())); }

    QString m_full;
};
} // namespace

AppCard::AppCard(const AppInfo& app, const QString& place, QWidget* parent)
    : QFrame(parent)
    , m_app(app)
    , m_place(place)
{
    setObjectName(u"ktechpitAppCard"_s);
    setFrameShape(QFrame::StyledPanel);
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("%1\n%2\nClick for details").arg(app.title, app.summary));
    setAttribute(Qt::WA_Hover);
    // Tab reaches the card, and Enter or Space opens its page, as a click does.
    setFocusPolicy(Qt::TabFocus);
    setAccessibleName(app.name);
    setAccessibleDescription(app.summary);

    m_icon = new QLabel(this);
    m_icon->setFixedSize(kIconSize, kIconSize);
    m_icon->setAlignment(Qt::AlignCenter);
    updateIcon();

    const bool oneLine = AppCatalog::instance()->config().oneLineText;
    QLabel* name = oneLine ? new OneLineLabel(app.name, this) : new QLabel(app.name, this);
    name->setObjectName(u"ktechpitAppName"_s); // object names for the host's style sheet
    QFont bold = name->font();
    bold.setBold(true);
    name->setFont(bold);

    QLabel* summary = oneLine ? static_cast<QLabel*>(new OneLineLabel(app.summary, this))
                              : new TwoLineLabel(app.summary, this);
    summary->setObjectName(u"ktechpitAppSummary"_s);
    summary->setForegroundRole(QPalette::PlaceholderText);

    if (AppLinks::isInstalled(app)) {
        m_get = new QPushButton(tr("Installed"), this);
        m_get->setEnabled(false);
        m_get->setToolTip(tr("%1 is installed on this computer").arg(app.name));
    } else {
        m_get = new QPushButton(tr("Get"), this);
        m_get->setCursor(Qt::PointingHandCursor);
        m_get->setToolTip(tr("Install %1").arg(app.name));
        connect(m_get, &QPushButton::clicked, this, [this] { AppLinks::openGet(m_app, m_place); });
    }

    m_get->setObjectName(u"ktechpitAppGet"_s);

    auto* text = new QVBoxLayout;
    text->setSpacing(2);
    text->addWidget(name);
    text->addWidget(summary);
    text->addStretch();

    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(10, 10, 10, 10);
    row->setSpacing(10);
    row->addWidget(m_icon, 0, Qt::AlignTop);
    row->addLayout(text, 1);
    row->addWidget(m_get, 0, Qt::AlignVCenter);

    connect(AppCatalog::instance(), &AppCatalog::iconReady, this, [this](const QString& id) {
        if (id == m_app.id) {
            updateIcon();
        }
    });
}

void AppCard::updateIcon()
{
    m_icon->setPixmap(AppCatalog::instance()->icon(m_app, kIconSize));
}

void AppCard::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->position().toPoint())) {
        AppLinks::openPage(m_app, m_place);
    }
    QFrame::mouseReleaseEvent(event);
}

void AppCard::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space) {
        AppLinks::openPage(m_app, m_place);
        event->accept();
        return;
    }
    QFrame::keyPressEvent(event);
}

void AppCard::enterEvent(QEnterEvent* event)
{
    setBackgroundRole(QPalette::AlternateBase);
    setAutoFillBackground(true);
    QFrame::enterEvent(event);
}

void AppCard::leaveEvent(QEvent* event)
{
    setAutoFillBackground(false);
    QFrame::leaveEvent(event);
}

} // namespace ktechpit
