#include "ui/lock_screen.h"

#include <QEasingCurve>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

LockScreen::LockScreen(QWidget* parent)
    : QWidget(parent)
    , m_doodle(u":/icons/wa_bg.png"_s)
{
    setAutoFillBackground(true); // opaque: never let the page show through

    // The visible content sits on a rounded card so it reads over the doodle
    // background; the card is what we animate in.
    m_card = new QFrame(this);
    m_card->setObjectName(u"lockCard"_s);
    m_card->setStyleSheet(u"QFrame#lockCard {"
                          u"  background-color: palette(base);"
                          u"  border: 1px solid palette(midlight);"
                          u"  border-radius: 16px;"
                          u"}"_s);

    auto* icon = new QLabel(m_card);
    icon->setPixmap(QIcon(u":/icons/whatsie.svg"_s).pixmap(72, 72));
    icon->setAlignment(Qt::AlignCenter);

    auto* title = new QLabel(tr("Whatsie is locked"), m_card);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPointSizeF(titleFont.pointSizeF() * 1.5);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_input = new QLineEdit(m_card);
    m_input->setEchoMode(QLineEdit::Password);
    m_input->setPlaceholderText(tr("Enter passcode"));
    m_input->setAlignment(Qt::AlignCenter);
    m_input->setMaxLength(128);
    m_input->setFixedWidth(240);
    connect(m_input, &QLineEdit::returnPressed, this, &LockScreen::submit);

    m_unlock = new QPushButton(tr("Unlock"), m_card);
    m_unlock->setProperty("whatsiePrimary", true);
    m_unlock->setFixedWidth(240);
    connect(m_unlock, &QPushButton::clicked, this, &LockScreen::submit);

    m_status = new QLabel(m_card);
    m_status->setAlignment(Qt::AlignCenter);
    m_status->setWordWrap(true);
    m_status->setStyleSheet(u"color: palette(placeholder-text);"_s);

    auto* card = new QVBoxLayout(m_card);
    card->setContentsMargins(40, 36, 40, 36);
    card->setSpacing(14);
    card->addWidget(icon);
    card->addWidget(title);
    card->addSpacing(6);
    card->addWidget(m_input, 0, Qt::AlignHCenter);
    card->addWidget(m_unlock, 0, Qt::AlignHCenter);
    card->addWidget(m_status);

    // Centre the card without stretching it across the whole view.
    auto* row = new QHBoxLayout;
    row->addStretch();
    row->addWidget(m_card);
    row->addStretch();
    auto* outer = new QVBoxLayout(this);
    outer->addStretch();
    outer->addLayout(row);
    outer->addStretch();

    // A light-recoloured copy of the doodle for dark themes: the shipped strokes
    // are grey and vanish on a dark ground, so keep the shapes and repaint them
    // near-white (SourceIn preserves the alpha, replaces the colour).
    if (!m_doodle.isNull()) {
        m_doodleLight = QPixmap(m_doodle.size());
        m_doodleLight.fill(Qt::transparent);
        QPainter tp(&m_doodleLight);
        tp.drawPixmap(0, 0, m_doodle);
        tp.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tp.fillRect(m_doodleLight.rect(), QColor(0xea, 0xed, 0xef));
        tp.end();
    }
}

void LockScreen::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    const QColor window = palette().color(QPalette::Window);
    p.fillRect(rect(), window);
    if (!m_doodle.isNull()) {
        // Lay the doodle on as a faint watermark: the grey original on a light
        // theme, the light-recoloured copy on a dark one, so the pattern reads
        // either way without glaring.
        const bool dark = window.lightness() < 128;
        p.setOpacity(dark ? 0.22 : 0.5);
        p.drawTiledPixmap(rect(), dark && !m_doodleLight.isNull() ? m_doodleLight : m_doodle);
    }
}

void LockScreen::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    animateIn();
}

void LockScreen::animateIn()
{
    if (m_card == nullptr) {
        return;
    }
    // Fade the card in while it rises a little into place — a calmer, more
    // deliberate reveal than the instant swap. The opacity effect is attached
    // only while animating and removed on completion: a QGraphicsEffect left on
    // a widget forces it to render through an offscreen pixmap, which can paint
    // black after a modal dialog (e.g. About) re-exposes the view.
    auto* effect = new QGraphicsOpacityEffect(m_card);
    m_card->setGraphicsEffect(effect);
    effect->setOpacity(0.0);
    auto* fade = new QPropertyAnimation(effect, "opacity", this);
    fade->setDuration(320);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->setEasingCurve(QEasingCurve::OutCubic);

    auto* group = new QParallelAnimationGroup(this);
    group->addAnimation(fade);

    // Only add the upward-glide when the card already has a real, laid-out
    // geometry; animating "geometry" from a zero rect (very first show, before
    // layout) would briefly collapse the card to nothing.
    const QRect end = m_card->geometry();
    if (end.isValid() && end.width() > 0 && end.height() > 0) {
        auto* rise = new QPropertyAnimation(m_card, "geometry", this);
        rise->setDuration(360);
        rise->setStartValue(end.translated(0, 18));
        rise->setEndValue(end);
        rise->setEasingCurve(QEasingCurve::OutCubic);
        group->addAnimation(rise);
    }
    // Render normally when idle — but only clear OUR effect, in case a dismiss
    // started in the meantime and installed its own.
    connect(group, &QParallelAnimationGroup::finished, this, [this, effect] {
        if (m_card->graphicsEffect() == effect) {
            m_card->setGraphicsEffect(nullptr);
        }
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void LockScreen::playDismiss()
{
    if (m_card == nullptr) {
        Q_EMIT dismissed();
        return;
    }
    // Fade the card away while it glides upward, then hand back so the app is
    // revealed — the mirror of animateIn(). The opacity effect is attached only
    // for the animation and dropped afterwards (see animateIn()).
    auto* effect = new QGraphicsOpacityEffect(m_card);
    m_card->setGraphicsEffect(effect);
    effect->setOpacity(1.0);
    auto* fade = new QPropertyAnimation(effect, "opacity", this);
    fade->setDuration(300);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);
    fade->setEasingCurve(QEasingCurve::InCubic);

    auto* group = new QParallelAnimationGroup(this);
    group->addAnimation(fade);

    const QRect start = m_card->geometry();
    if (start.isValid() && start.width() > 0 && start.height() > 0) {
        auto* glide = new QPropertyAnimation(m_card, "geometry", this);
        glide->setDuration(320);
        glide->setStartValue(start);
        glide->setEndValue(start.translated(0, -40));
        glide->setEasingCurve(QEasingCurve::InBack);
        group->addAnimation(glide);
    }
    connect(group, &QParallelAnimationGroup::finished, this, [this] {
        m_card->setGraphicsEffect(nullptr); // drop the effect; card is hidden next
        Q_EMIT dismissed();
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
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
