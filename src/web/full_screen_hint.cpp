#include "web/full_screen_hint.h"

#include <QEasingCurve>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace whatsie::web {

FullScreenHint::FullScreenHint(QWidget* host)
    : QLabel(host)
    , m_host(host)
{
    setObjectName(u"fullScreenHint"_s);
    setAttribute(Qt::WA_TransparentForMouseEvents); // clicks pass through to the page
    setStyleSheet(u"QLabel#fullScreenHint {"
                  u"  background-color: rgba(0, 0, 0, 190);"
                  u"  color: white;"
                  u"  padding: 8px 18px;"
                  u"  border-radius: 10px;"
                  u"}"_s);
    hide();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(3500);
    connect(m_timer, &QTimer::timeout, this, &FullScreenHint::hideHint);

    host->installEventFilter(this); // re-centre when the host resizes
}

void FullScreenHint::showHint(const QString& text)
{
    setText(text);
    setGraphicsEffect(nullptr); // clear any leftover fade
    reposition();
    show();
    raise();
    m_timer->start();
}

void FullScreenHint::hideHint()
{
    if (!isVisible() || graphicsEffect() != nullptr) {
        return; // hidden already, or a fade is already running
    }
    m_timer->stop();
    // The opacity effect is attached only for the fade and dropped afterwards (a
    // QGraphicsEffect left on a widget can force offscreen rendering that paints
    // it black on re-expose).
    auto* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    auto* fade = new QPropertyAnimation(effect, "opacity", this);
    fade->setDuration(400);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);
    fade->setEasingCurve(QEasingCurve::InCubic);
    connect(fade, &QPropertyAnimation::finished, this, [this, effect] {
        if (graphicsEffect() == effect) {
            setGraphicsEffect(nullptr);
        }
        hide();
    });
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

void FullScreenHint::reposition()
{
    adjustSize();
    move(std::max(0, (m_host->width() - width()) / 2), 28);
}

bool FullScreenHint::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_host && event->type() == QEvent::Resize && isVisible()) {
        reposition();
    }
    return QLabel::eventFilter(watched, event);
}

} // namespace whatsie::web
