#pragma once

#include <QPixmap>
#include <QWidget>

class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QGraphicsOpacityEffect;

namespace whatsie::ui {

/// Full-view lock cover shown in place of the web view while the app is locked
/// (FEATURES P1). It owns no policy: it emits the entered passcode and is told
/// what to display (error, throttle countdown) by the controller.
class LockScreen : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(LockScreen)

public:
    explicit LockScreen(QWidget* parent = nullptr);
    ~LockScreen() override = default;

    /// Clear the field, drop any error, enable input and focus it.
    void reset();
    void showError(const QString& message);
    /// Disable entry and show a countdown; call with 0 to re-enable.
    void showThrottle(int secondsRemaining);

Q_SIGNALS:
    void unlockRequested(const QString& passcode);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void submit();
    void animateIn();

    QFrame* m_card = nullptr;
    QGraphicsOpacityEffect* m_cardOpacity = nullptr;
    QLabel* m_status = nullptr;
    QLineEdit* m_input = nullptr;
    QPushButton* m_unlock = nullptr;
    QPixmap m_doodle;       ///< as-shipped grey doodles, for light themes
    QPixmap m_doodleLight;  ///< strokes recoloured light, for dark themes
};

} // namespace whatsie::ui
