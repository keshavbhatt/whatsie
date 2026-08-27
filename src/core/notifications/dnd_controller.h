#pragma once

#include <QDateTime>
#include <QObject>
#include <QTimer>

#include <chrono>
#include <optional>

namespace whatsie::core {

/// Manual Do-Not-Disturb (FEATURES N7): for a duration or until switched off.
/// Not persisted on purpose — a restart ends DND, which is the least
/// surprising behaviour for a manual switch.
class DndController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DndController)

public:
    explicit DndController(QObject* parent = nullptr);
    ~DndController() override = default;

    [[nodiscard]] bool isActive() const { return m_active; }
    /// Set when DND ends automatically; nullopt when indefinite or inactive.
    [[nodiscard]] std::optional<QDateTime> until() const { return m_until; }

    void enableFor(std::chrono::minutes duration);
    void enableIndefinitely();
    void disable();

Q_SIGNALS:
    void stateChanged(bool active);

private:
    void setActive(bool active);

    bool m_active = false;
    std::optional<QDateTime> m_until;
    QTimer m_timer;
};

} // namespace whatsie::core
