#include "core/timezone.h"

#include <QFile>
#include <QFileInfo>
#include <QTimeZone>

namespace whatsie::core {

namespace {

/// Returns the canonical IANA id, or an empty string when the candidate is not
/// a zone Qt knows. Qt also accepts Windows ids on Windows, which is fine: the
/// caller only exports TZ on Unix.
QString canonicalId(const QString& candidate)
{
    if (candidate.isEmpty()) {
        return {};
    }
    const QTimeZone zone(candidate.toUtf8());
    return zone.isValid() ? QString::fromUtf8(zone.id()) : QString();
}

} // namespace

TimeZoneChoice resolveTimeZone(const QString& overrideId, const QString& envTz,
                               const QString& systemId)
{
    TimeZoneChoice choice;

    const auto addWarning = [&choice](const QString& text) {
        if (!choice.warning.isEmpty()) {
            choice.warning += QStringLiteral("; ");
        }
        choice.warning += text;
    };

    // 1. An explicit choice from Settings wins. An invalid one is ignored so a
    //    stale or hand-edited value cannot make the problem worse.
    const QString overrideTrimmed = overrideId.trimmed();
    if (!overrideTrimmed.isEmpty()) {
        const QString canonical = canonicalId(overrideTrimmed);
        if (!canonical.isEmpty()) {
            choice.tzValue = canonical.toUtf8();
            choice.ianaId = canonical;
            choice.source = QStringLiteral("setting");
            return choice;
        }
        addWarning(QStringLiteral("ignoring invalid time-zone setting \"%1\"").arg(overrideTrimmed));
    }

    // 2. A TZ value from the environment. Keep a valid IANA id or a POSIX-style
    //    string (EST5EDT, UTC0, ...), but normalize the forms ICU misreads.
    const QString envTrimmed = envTz.trimmed();
    if (!envTrimmed.isEmpty()) {
        QString candidate = envTrimmed;
        bool normalized = false;
        if (candidate.startsWith(u':')) {
            candidate.remove(0, 1);
            normalized = true;
        }

        if (candidate.isEmpty()) {
            addWarning(QStringLiteral("ignoring empty TZ \"%1\"").arg(envTrimmed));
        } else if (candidate.startsWith(u'/')) {
            // e.g. TZ=:/etc/localtime or TZ=/etc/localtime. Chromium/ICU does
            // not reliably follow a path here and can fall back to UTC.
            addWarning(QStringLiteral("ignoring path-style TZ \"%1\"").arg(envTrimmed));
        } else {
            const QString canonical = canonicalId(candidate);
            if (!canonical.isEmpty()) {
                choice.tzValue = canonical.toUtf8();
                choice.ianaId = canonical;
                choice.source = normalized ? QStringLiteral("env-normalized") : QStringLiteral("env");
                return choice;
            }
            // POSIX TZ strings have no slash or space. They are a deliberate
            // expert choice, so leave them alone rather than guessing at
            // validity. (A Windows id leaking into a Unix TZ has a space.)
            if (!candidate.contains(u'/') && !candidate.contains(u' ')) {
                choice.tzValue = candidate.toUtf8();
                choice.ianaId = candidate;
                choice.source = normalized ? QStringLiteral("env-normalized") : QStringLiteral("env");
                return choice;
            }
            addWarning(QStringLiteral("ignoring invalid TZ \"%1\"").arg(envTrimmed));
        }
    }

    // 3. Fall back to the zone the desktop reports. Exporting it as a clean
    //    IANA id is the actual fix for a sandbox that handed Chromium a
    //    malformed or stale /etc/localtime.
    const QString canonicalSystem = canonicalId(systemId.trimmed());
    if (!canonicalSystem.isEmpty()) {
        choice.tzValue = canonicalSystem.toUtf8();
        choice.ianaId = canonicalSystem;
        choice.source = QStringLiteral("system");
        return choice;
    }

    choice.source = QStringLiteral("none");
    addWarning(QStringLiteral("no usable system time zone; leaving TZ unchanged"));
    return choice;
}

QString desktopTimeZoneId()
{
#ifdef Q_OS_WIN
    return QString::fromUtf8(QTimeZone::systemTimeZoneId());
#else
    // Prefer the /etc/localtime symlink: it is what libc (and therefore
    // Chromium's fallback path) reads. A copied file has no target, so fall
    // back to /etc/timezone (Debian and friends).
    const QString link = QFileInfo(QStringLiteral("/etc/localtime")).symLinkTarget();
    const qsizetype marker = link.indexOf(QStringLiteral("zoneinfo/"));
    if (marker >= 0) {
        QString id = link.mid(marker + 9);
        // Some distros link into zoneinfo/posix/ or zoneinfo/right/; the zone
        // id itself starts after that directory.
        if (id.startsWith(QStringLiteral("posix/"))) {
            id.remove(0, 6);
        } else if (id.startsWith(QStringLiteral("right/"))) {
            id.remove(0, 6);
        }
        const QString canonical = canonicalId(id);
        if (!canonical.isEmpty()) {
            return canonical;
        }
    }

    QFile timezoneFile(QStringLiteral("/etc/timezone"));
    if (timezoneFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString id = QString::fromUtf8(timezoneFile.readAll()).trimmed();
        const QString canonical = canonicalId(id);
        if (!canonical.isEmpty()) {
            return canonical;
        }
    }

    // Last resort. On a host where TZ is malformed this can echo that value,
    // but the two sources above cover the usual Linux/macOS layouts.
    return QString::fromUtf8(QTimeZone::systemTimeZoneId());
#endif
}

} // namespace whatsie::core
