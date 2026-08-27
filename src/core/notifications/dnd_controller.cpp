#include "core/notifications/dnd_controller.h"

#include "core/logging.h"

namespace whatsie::core {

DndController::DndController(QObject* parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &DndController::disable);
}

void DndController::enableFor(std::chrono::minutes duration)
{
    m_until = QDateTime::currentDateTime().addSecs(std::chrono::seconds(duration).count());
    m_timer.start(duration);
    qCInfo(lcCore) << "do not disturb until" << m_until->toString(Qt::ISODate);
    setActive(true);
}

void DndController::enableIndefinitely()
{
    m_timer.stop();
    m_until.reset();
    qCInfo(lcCore) << "do not disturb until switched off";
    setActive(true);
}

void DndController::disable()
{
    m_timer.stop();
    m_until.reset();
    if (m_active) {
        qCInfo(lcCore) << "do not disturb off";
    }
    setActive(false);
}

void DndController::setActive(bool active)
{
    if (active == m_active) {
        return;
    }
    m_active = active;
    Q_EMIT stateChanged(active);
}

} // namespace whatsie::core
