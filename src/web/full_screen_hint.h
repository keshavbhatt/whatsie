#pragma once

#include <QLabel>

class QTimer;

namespace whatsie::web {

/// A transient, Chrome-style pill overlaid on a host widget — e.g. "Press Esc to
/// exit full screen". Shows for a few seconds then fades, ignores the mouse, and
/// re-centres itself when the host resizes. Lives in the web layer so both the
/// main window (ui) and the call pop-out (web) can share one implementation.
class FullScreenHint : public QLabel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FullScreenHint)

public:
    explicit FullScreenHint(QWidget* host);
    ~FullScreenHint() override = default;

    void showHint(const QString& text);
    void hideHint();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void reposition();

    QWidget* m_host = nullptr;
    QTimer* m_timer = nullptr;
};

} // namespace whatsie::web
