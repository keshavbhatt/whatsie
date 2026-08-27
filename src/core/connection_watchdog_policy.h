#pragma once

#include <chrono>

// Decides when to reload WhatsApp Web after the connection drops (FEATURES S13,
// S14). Pure: time is injected so it can be unit-tested. Reloads only — never
// wipes data (contrast with whatsie's destructive watchdog, LESSONS A6).
//
// Model: the page reports connected/disconnected. While disconnected past a
// grace period the watchdog asks for a reload, at most `maxReloads` times per
// disconnection episode with a cooldown between; a reconnect ends the episode.
// A network-came-back signal (S14) forces one more attempt even after the cap.
namespace whatsie::core {

class ConnectionWatchdogPolicy
{
public:
    using ms = std::chrono::milliseconds;

    ConnectionWatchdogPolicy(ms grace = std::chrono::seconds(20), int maxReloads = 3,
                             ms cooldown = std::chrono::seconds(15));

    /// The page's connection state changed (from the injected watchdog script).
    void setConnected(bool connected, ms now);
    /// Reachability returned (QNetworkInformation): allow one more reload even
    /// if the cap was hit, since the cause may now be gone.
    void networkReturned(ms now);

    /// Should a reload happen at time `now`? Call on a timer while disconnected.
    [[nodiscard]] bool shouldReload(ms now) const;
    /// Record that a reload was issued (restarts the grace/cooldown).
    void noteReload(ms now);

    [[nodiscard]] bool isConnected() const { return m_connected; }
    [[nodiscard]] int reloadsThisEpisode() const { return m_reloads; }

private:
    ms m_grace;
    int m_maxReloads;
    ms m_cooldown;

    bool m_connected = true;
    ms m_downSince{-1}; // -1 = not down
    ms m_lastReload{-1};
    int m_reloads = 0;
};

} // namespace whatsie::core
