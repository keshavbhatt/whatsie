#pragma once

#include <QByteArray>
#include <QString>

namespace whatsie::core {

/// The result of normalizing the process time zone.
///
/// Chromium reads the zone once, at start-up, from the TZ environment variable
/// (falling back to /etc/localtime). It misreads some forms — a leading colon,
/// an absolute path such as /etc/localtime, or a stale sandbox copy — and can
/// end up an hour (or more) away from the desktop's zone. WhatsApp Web then
/// renders every message timestamp in that wrong zone (#403).
struct TimeZoneChoice
{
    /// Value to export as TZ. Empty means "leave the environment alone".
    QByteArray tzValue;
    /// Resolved zone, for logs and diagnostics.
    QString ianaId;
    /// Where the choice came from: "setting", "env", "env-normalized",
    /// "system" or "none".
    QString source;
    /// Human-readable note when an override or TZ value was ignored.
    QString warning;
};

/// Resolves the zone to hand to Chromium. Pure: it reads no environment and no
/// settings, so the rules are unit-testable.
///
/// Precedence: a valid `overrideId` wins, then a usable `envTz`, then
/// `systemId`. Path-style and invalid TZ values are rejected rather than
/// passed through, because those are what Chromium misparses.
[[nodiscard]] TimeZoneChoice resolveTimeZone(const QString& overrideId, const QString& envTz,
                                             const QString& systemId);

/// The desktop's zone, read from /etc/localtime or /etc/timezone instead of
/// from TZ. This matters when TZ itself is the broken value: asking Qt for the
/// "system" zone can otherwise echo the malformed TZ back. Returns an empty
/// string when no source is usable; on Windows it is QTimeZone's system id.
[[nodiscard]] QString desktopTimeZoneId();

} // namespace whatsie::core
