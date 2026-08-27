#pragma once

#include <QWebEnginePermission>

class QWidget;

namespace whatsie::ui {

/// Non-blocking Allow/Deny question for a web permission (FEATURES M1). The
/// answer is persisted by WebEngine; Settings → Privacy can reset it.
void askPermission(QWidget* parent, QWebEnginePermission permission);

} // namespace whatsie::ui
