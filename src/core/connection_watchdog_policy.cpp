#include "core/connection_watchdog_policy.h"

#include <algorithm>

namespace whatsie::core {

ConnectionWatchdogPolicy::ConnectionWatchdogPolicy(ms grace, int maxReloads, ms cooldown, ms slowRetry)
    : m_grace(grace)
    , m_maxReloads(std::max(1, maxReloads))
    , m_cooldown(cooldown)
    , m_slowRetry(slowRetry)
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
    // The OS says the link is back: make the page eligible to reload now by
    // treating the grace as already elapsed, and — if the reload cap was hit —
    // grant exactly one more attempt. The cooldown (m_lastReload) is deliberately
    // kept, so a connection that flaps Online repeatedly cannot reload on every
    // edge (a reload storm); shouldReload still enforces one reload per cooldown.
    m_downSince = now - m_grace;
    if (m_reloads >= m_maxReloads) {
        m_reloads = m_maxReloads - 1;
    }
}

bool ConnectionWatchdogPolicy::shouldReload(ms now) const
{
    if (m_connected || m_downSince.count() < 0) {
        return false;
    }
    if (now - m_downSince < m_grace) {
        return false;
    }
    // The first `maxReloads` attempts run at the fast `cooldown` cadence; after
    // that the watchdog keeps trying at the slower `slowRetry` cadence rather
    // than giving up, so a long outage (e.g. Wi-Fi still down on resume from
    // sleep) still recovers once the link returns, without needing the network-
    // return signal that some Linux/Wayland/Flatpak stacks never deliver (#370).
    const ms interval = m_reloads >= m_maxReloads ? m_slowRetry : m_cooldown;
    if (m_lastReload.count() >= 0 && now - m_lastReload < interval) {
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
