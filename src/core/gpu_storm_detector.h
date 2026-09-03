#pragma once

#include <QString>

#include <cstdint>
#include <deque>

namespace whatsie::core {

/// Detects a Chromium GPU "context lost" storm — the EGL_BAD_DISPLAY flood that
/// hangs the app when a Wayland screen share is stopped, as the GPU process
/// loses its context on the vendor driver and retries forever. Pure and
/// windowed so it is unit-tested; fires once. Feed it every stderr line.
class GpuStormDetector
{
public:
    static constexpr int kWindowMs = 3000;  ///< rolling window
    static constexpr int kThreshold = 50;   ///< matching lines within the window → storm

    /// Returns true the first (and only) time the storm threshold is crossed.
    bool observe(std::int64_t nowMs, const QString& line);
    [[nodiscard]] bool fired() const { return m_fired; }

    /// Whether a line is one of the GPU-context-loss markers (a transient burst
    /// of a few is normal; a sustained flood is the hang).
    [[nodiscard]] static bool isContextLossLine(const QString& line);

private:
    std::deque<std::int64_t> m_hits;
    bool m_fired = false;
};

} // namespace whatsie::core
