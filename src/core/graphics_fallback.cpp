#include "core/graphics_fallback.h"

#include <array>

namespace whatsie::core {

bool isGraphicsInitFailure(const QString& message)
{
    // Curated so ordinary GPU chatter does not trigger a relaunch — each marker
    // is emitted by Qt/Chromium only when the backend genuinely failed to start.
    static constexpr std::array<QLatin1StringView, 8> kMarkers{
        QLatin1StringView("Failed to create platform opengl context"),
        QLatin1StringView("Failed to create OpenGL context"),
        QLatin1StringView("Failed to initialize graphics backend"),
        QLatin1StringView("Failed to create RHI"),
        QLatin1StringView("Failed to create QRhi"),
        QLatin1StringView("eglInitialize failed"),
        QLatin1StringView("Could not initialize GLX"),
        QLatin1StringView("Failed to create OpenGL context for format"),
    };
    for (const QLatin1StringView marker : kMarkers) {
        if (message.contains(marker, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool isGbmVulkanFallback(const QString& message)
{
    // Chromium/ozone emits this on the NVIDIA driver under Wayland when it cannot
    // allocate GBM buffers; the Vulkan fallback then renders the page black.
    return message.contains(QLatin1StringView("GBM is not supported"), Qt::CaseInsensitive);
}

bool shouldRetryUnderXcb(QLatin1StringView platformName, bool alreadyRetried, bool failureSeen)
{
    return failureSeen && !alreadyRetried && platformName == QLatin1StringView("wayland");
}

} // namespace whatsie::core
