#pragma once

#include "core/notifications/notification.h"

namespace whatsie::ui {

class TrayController;

/// Fallback backend using QSystemTrayIcon::showMessage — the native path on
/// Windows (FEATURES N2) and the last resort on Linux without a notification
/// daemon. Only one balloon is visible at a time, so activation maps to the
/// most recently shown id.
class TrayNotifier : public core::INotifier
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(TrayNotifier)

public:
    explicit TrayNotifier(TrayController& tray, QObject* parent = nullptr);
    ~TrayNotifier() override = default;

    [[nodiscard]] QString name() const override { return QStringLiteral("tray"); }
    [[nodiscard]] bool isAvailable() const override;
    void show(quint64 id, const core::Notification& notification) override;
    void close(quint64 id) override;

private:
    TrayController& m_tray;
    quint64 m_current = 0;
};

} // namespace whatsie::ui
