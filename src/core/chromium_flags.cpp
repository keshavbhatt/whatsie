#include "core/chromium_flags.h"

#include "core/settings/settings.h"

#include <QHash>

using namespace Qt::StringLiterals;

namespace whatsie::core {

QStringList chromiumFlags(HardwareAcceleration acceleration)
{
    QStringList flags{
        // Nothing here is a chat client's business.
        u"--disable-translate"_s,
        u"--disable-extensions"_s,
        u"--disable-component-update"_s,
        u"--disable-default-apps"_s,
    };
    QStringList features{
        // WhatsApp Web's calling stack requires SharedArrayBuffer. Chrome gets
        // it on web.whatsapp.com through a Chrome-only origin trial (the page
        // is not cross-origin isolated: COOP same-origin-allow-popups), which
        // QtWebEngine does not honour → "Your browser doesn't support calling".
        // Verified 2026-08-27 by reading WAWebVoipGatingUtils.
        u"SharedArrayBuffer"_s,
    };
#ifdef Q_OS_LINUX
    // Screen sharing on Wayland goes through PipeWire (FEATURES M2).
    features << u"WebRTCPipeWireCapturer"_s;
#endif
    flags << u"--enable-features="_s + features.join(u',');
    switch (acceleration) {
    case HardwareAcceleration::Auto:
        break;
    case HardwareAcceleration::On:
        flags << u"--ignore-gpu-blocklist"_s;
        break;
    case HardwareAcceleration::Off:
        flags << u"--disable-gpu"_s;
        break;
    }
    return flags;
}

namespace {

// Chromium keeps only the last --enable-features/--disable-features switch,
// so their comma lists must be merged instead of appended.
bool isFeatureList(const QString& flag, QString* key, QStringList* values)
{
    for (const auto& prefix : {u"--enable-features="_s, u"--disable-features="_s}) {
        if (flag.startsWith(prefix)) {
            *key = prefix;
            *values = flag.mid(prefix.size()).split(u',', Qt::SkipEmptyParts);
            return true;
        }
    }
    return false;
}

} // namespace

QString mergeChromiumFlags(const QString& userFlags, const QStringList& ours)
{
    QStringList merged;
    QHash<QString, QStringList> featureLists; // prefix → values, in first-seen order
    QStringList featureOrder;
    auto absorb = [&](const QString& flag) {
        QString key;
        QStringList values;
        if (isFeatureList(flag, &key, &values)) {
            if (!featureLists.contains(key)) {
                featureOrder << key;
            }
            for (const QString& v : values) {
                if (!featureLists[key].contains(v)) {
                    featureLists[key] << v;
                }
            }
            return;
        }
        if (!merged.contains(flag)) {
            merged << flag;
        }
    };
    for (const QString& flag : userFlags.split(u' ', Qt::SkipEmptyParts)) {
        absorb(flag);
    }
    for (const QString& flag : ours) {
        absorb(flag);
    }
    for (const QString& key : featureOrder) {
        merged << key + featureLists.value(key).join(u',');
    }
    return merged.join(u' ');
}

} // namespace whatsie::core
