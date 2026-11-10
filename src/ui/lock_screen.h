#pragma once

#include <QPixmap>
#include <QWidget>

class QAnimationGroup;
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;

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
    /// Show the "More apps from Ktechpit" link under the card, which opens the full
    /// list. Hidden when the catalog is off or empty.
    void setOtherAppsVisible(bool visible);

    /// Play the dismissal animation (card fades away); emits dismissed() when the
    /// animation ends, at which point the caller reveals the app.
    void playDismiss();

Q_SIGNALS:
    void unlockRequested(const QString& passcode);
    /// The dismissal animation has finished.
    void dismissed();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void submit();
    void animateIn();
    void updateOtherAppsVisibility();
    void openOtherApps();
    /// Fades the link along with the card, so it never lingers on its own.
    void fadeOtherApps(QAnimationGroup* group, qreal from, qreal to, int duration);

    QFrame* m_card = nullptr;
    QLabel* m_status = nullptr;
    QLineEdit* m_input = nullptr;
    QPushButton* m_unlock = nullptr;
    QPushButton* m_appsLink = nullptr;
    bool m_appsWanted = false;
    QPixmap m_doodle;       ///< as-shipped grey doodles, for light themes
    QPixmap m_doodleLight;  ///< strokes recoloured light, for dark themes
};

} // namespace whatsie::ui
