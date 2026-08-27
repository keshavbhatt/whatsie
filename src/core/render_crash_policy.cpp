#include "core/render_crash_policy.h"

#include <algorithm>

namespace whatsie::core {

RenderCrashPolicy::RenderCrashPolicy(int maxRetries, std::chrono::milliseconds window)
    : m_maxRetries(std::max(1, maxRetries))
    , m_window(window)
{}

RenderCrashPolicy::Decision RenderCrashPolicy::onCrash(std::chrono::milliseconds now)
{
    m_crashTimes.removeIf([&](std::chrono::milliseconds t) { return now - t > m_window; });
    m_crashTimes.append(now);

    const auto attempt = static_cast<int>(m_crashTimes.size());
    if (attempt > m_maxRetries) {
        return {};
    }
    // 1 s, 3 s, 9 s, ...
    std::chrono::milliseconds delay{1000};
    for (int i = 1; i < attempt; ++i) {
        delay *= 3;
    }
    return {.reload = true, .delay = delay};
}

void RenderCrashPolicy::onLoadSucceeded()
{
    m_crashTimes.clear();
}

int RenderCrashPolicy::crashesInWindow() const
{
    return static_cast<int>(m_crashTimes.size());
}

} // namespace whatsie::core
