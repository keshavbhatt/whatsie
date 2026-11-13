#include "core/timezone.h"

namespace whatsie::core {

QString formatUtcOffset(std::optional<int> seconds)
{
    if (!seconds.has_value() || *seconds < -24 * 60 * 60 || *seconds > 24 * 60 * 60 ||
        *seconds % 60 != 0) {
        return QStringLiteral("unavailable");
    }
    const int value = *seconds;
    const int minutes = (value < 0 ? -value : value) / 60;
    return QStringLiteral("%1%2:%3")
        .arg(value < 0 ? QLatin1Char('-') : QLatin1Char('+'))
        .arg(minutes / 60, 2, 10, QLatin1Char('0'))
        .arg(minutes % 60, 2, 10, QLatin1Char('0'));
}

} // namespace whatsie::core
