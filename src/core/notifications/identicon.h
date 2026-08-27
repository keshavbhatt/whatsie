#pragma once

#include <QColor>
#include <QImage>
#include <QString>

// Letter-tile avatar for contacts without a picture (FEATURES D5). Colours are
// derived from a stable hash so a contact keeps its colour across launches
// (whatly used the randomly seeded qHash).
namespace whatsie::core {

[[nodiscard]] QColor identiconColor(const QString& label);
[[nodiscard]] QString identiconInitials(const QString& label);
[[nodiscard]] QImage identicon(const QString& label, int size = 128);

} // namespace whatsie::core
