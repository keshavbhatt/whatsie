#include "platform/gpu_stderr_watch.h"

#include <QDateTime>
#include <QSocketNotifier>

#include <fcntl.h>
#include <unistd.h>

namespace whatsie::platform {

GpuStderrWatch::GpuStderrWatch(QObject* parent)
    : QObject(parent)
{}

GpuStderrWatch::~GpuStderrWatch()
{
    if (m_savedStderr >= 0) {
        ::dup2(m_savedStderr, STDERR_FILENO); // restore so later stderr still works
        ::close(m_savedStderr);
    }
    if (m_readFd >= 0) {
        ::close(m_readFd);
    }
}

bool GpuStderrWatch::install()
{
    int fds[2];
    if (::pipe(fds) != 0) {
        return false;
    }
    m_savedStderr = ::dup(STDERR_FILENO);
    if (m_savedStderr < 0) {
        ::close(fds[0]);
        ::close(fds[1]);
        return false;
    }
    ::dup2(fds[1], STDERR_FILENO); // fd 2 is now the pipe; subprocesses inherit it
    ::close(fds[1]);
    m_readFd = fds[0];
    ::fcntl(m_readFd, F_SETFL, O_NONBLOCK);

    m_notifier = new QSocketNotifier(m_readFd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &GpuStderrWatch::onReadable);
    return true;
}

void GpuStderrWatch::onReadable()
{
    char chunk[4096];
    ssize_t n = 0;
    while ((n = ::read(m_readFd, chunk, sizeof chunk)) > 0) {
        // Pass everything through to the real stderr, unchanged, so logs still
        // appear exactly as before.
        ssize_t off = 0;
        while (off < n) {
            const ssize_t w = ::write(m_savedStderr, chunk + off, static_cast<size_t>(n - off));
            if (w <= 0) {
                break;
            }
            off += w;
        }
        m_buffer.append(chunk, static_cast<int>(n));
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    qsizetype nl = 0;
    while ((nl = m_buffer.indexOf('\n')) >= 0) {
        const QString line = QString::fromUtf8(m_buffer.constData(), nl);
        m_buffer.remove(0, nl + 1);
        if (m_detector.observe(now, line)) {
            Q_EMIT gpuContextLostStorm();
        }
    }
    if (m_buffer.size() > 65536) {
        m_buffer.clear(); // a single runaway line without a newline — do not grow
    }
}

} // namespace whatsie::platform
