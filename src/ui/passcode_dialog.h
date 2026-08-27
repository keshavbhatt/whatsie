#pragma once

#include "core/app_lock.h"

#include <QDialog>

class QLabel;
class QLineEdit;

namespace whatsie::ui {

/// Sets or changes the app-lock passcode (FEATURES P1). When a passcode already
/// exists the dialog first verifies the current one; it only accepts when the
/// new entry and its confirmation match. It never persists anything — the caller
/// turns newPasscode() into a record via core::makePasscode.
class PasscodeDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(PasscodeDialog)

public:
    explicit PasscodeDialog(const core::PasscodeRecord& existing, QWidget* parent = nullptr);
    ~PasscodeDialog() override = default;

    [[nodiscard]] QString newPasscode() const;

private:
    void tryAccept();

    core::PasscodeRecord m_existing;
    QLineEdit* m_current = nullptr;
    QLineEdit* m_new = nullptr;
    QLineEdit* m_confirm = nullptr;
    QLabel* m_error = nullptr;
};

} // namespace whatsie::ui
