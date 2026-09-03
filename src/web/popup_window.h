#pragma once

#include <QWidget>

class QLabel;
class QShortcut;
class QTimer;
class QWebEnginePage;
class QWebEngineView;

namespace whatsie::web {

class WebProfile;

/// Hosts window.open() targets that stay on web.whatsapp.com — the call
/// window (FEATURES M3). External targets are sent to the browser and the
/// window closes itself. Esc / close button always work (W#333).
class PopupWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(PopupWindow)

public:
    explicit PopupWindow(WebProfile& profile, QWidget* parent = nullptr);
    ~PopupWindow() override = default;

    [[nodiscard]] QWebEnginePage* page() const;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    class Page;
    void exitFullScreen();
    void showFullScreenHint();
    void hideFullScreenHint();
    void positionFullScreenHint();
    QWebEngineView* m_view = nullptr;
    QShortcut* m_exitFullScreen = nullptr;
    Qt::WindowStates m_stateBeforeFullScreen = Qt::WindowNoState;
    QLabel* m_fullScreenHint = nullptr;
    QTimer* m_fullScreenHintTimer = nullptr;
};

} // namespace whatsie::web
