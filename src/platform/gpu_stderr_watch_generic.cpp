#include "platform/gpu_stderr_watch.h"

namespace whatsie::platform {

// Non-Linux: no stderr redirection (the storm is a Linux/Wayland GPU issue).
GpuStderrWatch::GpuStderrWatch(QObject* parent)
    : QObject(parent)
{}

GpuStderrWatch::~GpuStderrWatch() = default;

bool GpuStderrWatch::install()
{
    return false;
}

void GpuStderrWatch::onReadable() {}

} // namespace whatsie::platform
