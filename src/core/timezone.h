#pragma once

#include <QString>

#include <optional>

namespace whatsie::core {

/// Formats a UTC offset in seconds as ±HH:MM, or "unavailable" when absent.
[[nodiscard]] QString formatUtcOffset(std::optional<int> seconds);

} // namespace whatsie::core
