#pragma once

#include <QSize>

// Pure zoom rules shared by settings, the view and the window (FEATURES A6).
namespace whatsie::core {

inline constexpr double kMinZoom = 0.25;
inline constexpr double kMaxZoom = 5.0;
inline constexpr double kDefaultZoom = 1.0;
inline constexpr double kZoomStep = 0.1;

[[nodiscard]] double clampZoom(double factor);
[[nodiscard]] double zoomIn(double factor);
[[nodiscard]] double zoomOut(double factor);

/// Minimum window size grows with zoom so WhatsApp's layout never gets
/// squeezed below its own minimum (W#192).
[[nodiscard]] QSize scaledMinimumSize(const QSize& baseMinimum, double factor);

} // namespace whatsie::core
