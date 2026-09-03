#include "core/chromium_flags.h"

#include "core/settings/settings.h"

#include <QHash>

using namespace Qt::StringLiterals;

namespace whatsie::core {

bool useSoftwareGpu(HardwareAcceleration acceleration, bool autoDisabled)
{
    switch (acceleration) {
    case HardwareAcceleration::On:
        return false;
    case HardwareAcceleration::Off:
        return true;
    case HardwareAcceleration::Auto:
        break;
    }
    return autoDisabled;
}

QStringList chromiumFlags(HardwareAcceleration acceleration, bool gpuAutoDisabled)
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
    // Decode video on the CPU. Hardware decode hands the compositor NV12 frames
    // in a platform GPU-memory-buffer (dmabuf) that many Linux Mesa/Wayland
    // stacks — especially under snap/flatpak confinement — cannot back into the
    // shared GL context; the context is then lost and video goes blank while
    // audio continues (owner-reproduced on the snap; --disable-features=
    // VaapiVideoDecoder and --disable-gpu-memory-buffer-video-frames did NOT
    // help, only this did). GPU compositing and WebGL stay on, so calls still
    // render; the cost is CPU video decode, negligible for a chat client.
    // ADR-032. Expert override: set QTWEBENGINE_CHROMIUM_FLAGS.
    flags << u"--disable-accelerated-video-decode"_s;
#endif
    flags << u"--enable-features="_s + features.join(u',');
    if (useSoftwareGpu(acceleration, gpuAutoDisabled)) {
        // Fully disable the GPU process. SwiftShader (software compositing) was
        // tried first to keep WebGL, but it still spins up a GPU process that
        // touches the vendor EGL driver for the Wayland screen-capture dmabuf
        // import — which floods EGL_BAD_DISPLAY and hangs the app when a screen
        // share is stopped (owner-reproduced on KDE/Wayland; SwiftShader did NOT
        // help, only --disable-gpu did). Removing the GPU process entirely avoids
        // the vendor driver; WhatsApp calls still render (their remote video is a
        // <video> element fed by WebRTC, not WebGL — verified on a real call).
        // ADR-032.
        flags << u"--disable-gpu"_s;
    } else if (acceleration == HardwareAcceleration::On) {
        flags << u"--ignore-gpu-blocklist"_s;
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
