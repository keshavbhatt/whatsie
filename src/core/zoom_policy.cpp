#include "core/zoom_policy.h"

#include <algorithm>
#include <cmath>

namespace whatsie::core {

namespace {
double roundToStep(double value)
{
    // Keep two decimals so repeated in/out round-trips land on the same values.
    return std::round(value * 100.0) / 100.0;
}
} // namespace

double clampZoom(double factor)
{
    if (std::isnan(factor)) {
        return kDefaultZoom;
    }
    return std::clamp(factor, kMinZoom, kMaxZoom);
}

double zoomIn(double factor)
{
    return clampZoom(roundToStep(factor + kZoomStep));
}

double zoomOut(double factor)
{
    return clampZoom(roundToStep(factor - kZoomStep));
}

QSize scaledMinimumSize(const QSize& baseMinimum, double factor)
{
    const double f = clampZoom(factor);
    return {static_cast<int>(std::lround(baseMinimum.width() * f)),
            static_cast<int>(std::lround(baseMinimum.height() * f))};
}

} // namespace whatsie::core
