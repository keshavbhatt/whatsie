#include "platform/gpu_stderr_watch.h"

#include <QByteArray>
#include <QDateTime>
#include <QMetaObject>

#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

namespace whatsie::platform {

GpuStderrWatch::GpuStderrWatch(QObject* parent)
    : QObject(parent)
{}

GpuStderrWatch::~GpuStderrWatch()
{
    if (m_savedStderr >= 0) {
        ::dup2(m_savedStderr, STDERR_FILENO); // restore so later stderr still works
    }
    if (m_reader.joinable()) {
        // Subprocesses may still hold the pipe's write end, so the reader cannot
        // rely on end-of-file: wake it through the stop pipe instead.
        const char byte = 1;
        const ssize_t ignored = ::write(m_stopFds[1], &byte, 1);
        static_cast<void>(ignored);
        m_reader.join();
    }
    for (const int fd : {m_readFd, m_savedStderr, m_stopFds[0], m_stopFds[1]}) {
        if (fd >= 0) {
            ::close(fd);
        }
    }
}

bool GpuStderrWatch::install()
{
    int fds[2];
    if (::pipe2(fds, O_CLOEXEC) != 0) {
        return false;
    }
    if (::pipe2(m_stopFds, O_CLOEXEC) != 0) {
        ::close(fds[0]);
        ::close(fds[1]);
        return false;
    }
    m_savedStderr = ::fcntl(STDERR_FILENO, F_DUPFD_CLOEXEC, 0);
    if (m_savedStderr < 0) {
        for (const int fd : {fds[0], fds[1], m_stopFds[0], m_stopFds[1]}) {
            ::close(fd);
        }
        m_stopFds[0] = m_stopFds[1] = -1;
        return false;
    }
    m_readFd = fds[0];
    // Drain on a thread of our own: the pipe must never fill up waiting for the
    // GUI thread, or every writer (the GUI thread's own log lines included)
    // blocks and the app freezes during start-up (#387).
    m_reader = std::thread([this] { readLoop(); });
    ::dup2(fds[1], STDERR_FILENO); // fd 2 is now the pipe; subprocesses inherit it
    ::close(fds[1]);
    return true;
}

void GpuStderrWatch::readLoop()
{
    QByteArray buffer;
    char chunk[4096];
    pollfd fds[2] = {{m_readFd, POLLIN, 0}, {m_stopFds[0], POLLIN, 0}};
    for (;;) {
        if (::poll(fds, 2, -1) < 0) {
            if (errno == EINTR) {
                continue;
            }
            return;
        }
        if (fds[1].revents != 0) {
            return;
        }
        const ssize_t n = ::read(m_readFd, chunk, sizeof chunk);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n <= 0) {
            return; // every writer is gone
        }
        // Pass everything through to the real stderr, unchanged, so logs still
        // appear exactly as before.
        ssize_t off = 0;
        while (off < n) {
            const ssize_t w = ::write(m_savedStderr, chunk + off, static_cast<size_t>(n - off));
            if (w < 0 && errno == EINTR) {
                continue;
            }
            if (w <= 0) {
                break;
            }
            off += w;
        }
        buffer.append(chunk, static_cast<qsizetype>(n));

        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        qsizetype nl = 0;
        while ((nl = buffer.indexOf('\n')) >= 0) {
            const QString line = QString::fromUtf8(buffer.constData(), nl);
            buffer.remove(0, nl + 1);
            if (m_detector.observe(now, line)) {
                QMetaObject::invokeMethod(this, [this] { Q_EMIT gpuContextLostStorm(); }, Qt::QueuedConnection);
            }
        }
        if (buffer.size() > 65536) {
            buffer.clear(); // a single runaway line without a newline: do not grow
        }
    }
}

} // namespace whatsie::platform
