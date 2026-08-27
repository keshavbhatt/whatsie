#include "core/connection_watchdog_policy.h"

#include <algorithm>

namespace whatsie::core {

ConnectionWatchdogPolicy::ConnectionWatchdogPolicy(ms grace, int maxReloads, ms cooldown)
    : m_grace(grace)
    , m_maxReloads(std::max(1, maxReloads))
    , m_cooldown(cooldown)
{}

void ConnectionWatchdogPolicy::setConnected(bool connected, ms now)
{
    if (connected == m_connected) {
        return;
    }
    m_connected = connected;
    if (connected) {
        // Reconnected: episode over.
        m_downSince = ms{-1};
        m_reloads = 0;
        m_lastReload = ms{-1};
    } else {
        m_downSince = now;
    }
}

void ConnectionWatchdogPolicy::networkReturned(ms now)
{
    if (m_connected) {
        return;
    }
    // Give the page a fresh chance: reset the reload budget and grace so a
    // reload fires promptly.
    m_reloads = 0;
    m_lastReload = ms{-1};
    m_downSince = now - m_grace; // grace already elapsed → reload on next check
}

bool ConnectionWatchdogPolicy::shouldReload(ms now) const
{
    if (m_connected || m_downSince.count() < 0) {
        return false;
    }
    if (m_reloads >= m_maxReloads) {
        return false;
    }
    if (now - m_downSince < m_grace) {
        return false;
    }
    if (m_lastReload.count() >= 0 && now - m_lastReload < m_cooldown) {
        return false;
    }
    return true;
}

void ConnectionWatchdogPolicy::noteReload(ms now)
{
    ++m_reloads;
    m_lastReload = now;
    m_downSince = now; // require another grace period before the next reload
}

} // namespace whatsie::core
