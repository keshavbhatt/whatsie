#pragma once

#include <QList>

#include <chrono>

// Decides what to do when Chromium's render process dies (FEATURES S12).
// Pure: time is injected so it can be unit-tested. Reloads with growing
// delays; after `maxRetries` crashes inside `window` it gives up and lets the
// UI ask the user instead of looping (W#102, Y#28).
namespace whatsie::core {

class RenderCrashPolicy
{
public:
    struct Decision
    {
        bool reload = false;
        std::chrono::milliseconds delay{0};
    };

    explicit RenderCrashPolicy(int maxRetries = 3,
                               std::chrono::milliseconds window = std::chrono::minutes(2));

    [[nodiscard]] Decision onCrash(std::chrono::milliseconds now);
    /// A successful load ends the current crash episode.
    void onLoadSucceeded();
    [[nodiscard]] int crashesInWindow() const;

private:
    int m_maxRetries;
    std::chrono::milliseconds m_window;
    QList<std::chrono::milliseconds> m_crashTimes;
};

} // namespace whatsie::core
