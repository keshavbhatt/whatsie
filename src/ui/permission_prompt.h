#pragma once

#include <QWebEnginePermission>

#include <functional>

class QWidget;

namespace whatsie::ui {

/// Non-blocking Allow/Deny question for a web permission (FEATURES M1).
/// `answer(true|false)` is invoked exactly once with the user's choice.
void askPermission(QWidget* parent, const QWebEnginePermission& permission, std::function<void(bool)> answer);

} // namespace whatsie::ui
