#include "ui/actions.h"

#include <QAction>
#include <QKeySequence>
#include <QWidget>

namespace whatsie::ui {

namespace {

QAction* make(QWidget* owner, const QString& text, const QKeySequence& shortcut = {})
{
    auto* action = new QAction(text, owner);
    if (!shortcut.isEmpty()) {
        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::WindowShortcut);
    }
    // Registered on the window so shortcuts work while the web view has focus.
    owner->addAction(action);
    return action;
}

} // namespace

Actions::Actions(QWidget* owner)
    : QObject(owner)
{
    showHide = make(owner, tr("Hide to tray"), QKeySequence::Close); // Ctrl+W
    newChat = make(owner, tr("New chat…"), QKeySequence::New);       // Ctrl+N
    reload = make(owner, tr("Reload"), QKeySequence::Refresh);       // F5
    zoomIn = make(owner, tr("Zoom in"), QKeySequence::ZoomIn);
    zoomOut = make(owner, tr("Zoom out"), QKeySequence::ZoomOut);
    zoomReset = make(owner, tr("Reset zoom"), QKeySequence(Qt::CTRL | Qt::Key_0));
    fullScreen = make(owner, tr("Full screen"), QKeySequence::FullScreen); // F11
    settings = make(owner, tr("Settings…"), QKeySequence(Qt::CTRL | Qt::Key_Comma));
    about = make(owner, tr("About Whatsie"));
    quit = make(owner, tr("Quit"), QKeySequence::Quit); // Ctrl+Q

    fullScreen->setCheckable(true);
    quit->setMenuRole(QAction::QuitRole);
    about->setMenuRole(QAction::AboutRole);
    settings->setMenuRole(QAction::PreferencesRole);
}

QList<QAction*> Actions::all() const
{
    return {showHide, newChat, reload, zoomIn, zoomOut, zoomReset, fullScreen, settings, about, quit};
}

} // namespace whatsie::ui
