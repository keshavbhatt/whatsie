#pragma once

#include <QDialog>

namespace whatsie::ui {

class Actions;

/// Read-only list of keyboard shortcuts, generated from the shared Actions so
/// it can never drift from the real bindings (FEATURES S26).
class ShortcutsDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ShortcutsDialog)

public:
    ShortcutsDialog(const Actions& actions, QWidget* parent = nullptr);
    ~ShortcutsDialog() override = default;
};

} // namespace whatsie::ui
