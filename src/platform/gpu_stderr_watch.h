#pragma once

#include "core/gpu_storm_detector.h"

#include <QByteArray>
#include <QObject>

class QSocketNotifier;

namespace whatsie::platform {

/// Watches stderr for a GPU context-loss storm. Chromium's GPU subprocess
/// prints its EGL errors straight to fd 2, bypassing Qt's message handler, so
/// we redirect stderr through a pipe (forwarding everything to the real stderr
/// unchanged) and scan it. Linux only; a no-op stub elsewhere. Emits
/// gpuContextLostStorm() once, on the GUI thread.
class GpuStderrWatch : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(GpuStderrWatch)

public:
    explicit GpuStderrWatch(QObject* parent = nullptr);
    ~GpuStderrWatch() override;

    /// Redirect stderr and start watching. Call once, after QApplication exists
    /// and before the web engine spawns its subprocesses. Returns false (and
    /// leaves stderr untouched) if redirection is unavailable.
    bool install();

Q_SIGNALS:
    void gpuContextLostStorm();

private:
    void onReadable();

    core::GpuStormDetector m_detector;
    QSocketNotifier* m_notifier = nullptr;
    int m_readFd = -1;
    int m_savedStderr = -1;
    QByteArray m_buffer;
};

} // namespace whatsie::platform
