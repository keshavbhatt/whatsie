#pragma once

#include <QLatin1StringView>

// Every persisted key lives here exactly once. Format: "section/camelCase".
// Adding a key = adding a typed accessor pair on core::Settings + a test.
namespace whatsie::core::keys {

inline constexpr QLatin1StringView kWindowGeometry{"window/geometry"};
inline constexpr QLatin1StringView kWindowState{"window/state"};
inline constexpr QLatin1StringView kZoomFactor{"view/zoomFactor"};
inline constexpr QLatin1StringView kTheme{"appearance/theme"};

} // namespace whatsie::core::keys
