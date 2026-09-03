#include "core/gpu_storm_detector.h"

namespace whatsie::core {

bool GpuStormDetector::isContextLossLine(const QString& line)
{
    return line.contains(QLatin1StringView("EGL_BAD_DISPLAY")) ||
           line.contains(QLatin1StringView("context is marked as lost")) ||
           line.contains(QLatin1StringView("Failed to create SharedImageStub"));
}

bool GpuStormDetector::observe(std::int64_t nowMs, const QString& line)
{
    if (m_fired || !isContextLossLine(line)) {
        return false;
    }
    m_hits.push_back(nowMs);
    while (!m_hits.empty() && nowMs - m_hits.front() > kWindowMs) {
        m_hits.pop_front();
    }
    if (static_cast<int>(m_hits.size()) >= kThreshold) {
        m_fired = true;
        return true;
    }
    return false;
}

} // namespace whatsie::core
