#include "core/chromium_flags.h"

#include "core/settings/settings.h"

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
#ifdef Q_OS_LINUX
    // Screen sharing on Wayland goes through PipeWire (FEATURES M2).
    flags << u"--enable-features=WebRTCPipeWireCapturer"_s;
#endif
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

QString mergeChromiumFlags(const QString& userFlags, const QStringList& ours)
{
    QStringList merged = userFlags.split(u' ', Qt::SkipEmptyParts);
    for (const QString& flag : ours) {
        if (!merged.contains(flag)) {
            merged << flag;
        }
    }
    return merged.join(u' ');
}

} // namespace whatsie::core
