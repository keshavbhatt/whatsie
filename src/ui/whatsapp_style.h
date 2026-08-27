#pragma once

#include <QString>

namespace whatsie::ui {

/// A Qt style sheet that dresses the app's widgets in WhatsApp Web's design
/// language — green accent, rounded inputs, underlined tabs, pill checkboxes —
/// in a light or dark variant. Applied on top of the Fusion style and the
/// WhatsApp palette (ThemeApplier). Colours mirror WhatsApp Web's tokens.
[[nodiscard]] QString whatsappStyleSheet(bool dark);

} // namespace whatsie::ui
