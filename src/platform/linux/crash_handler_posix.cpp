#include "platform/crash_handler.h"

#include <QByteArray>
#include <QFile>

#include <array>
#include <csignal>
#include <cstring>
#include <execinfo.h>
#include <fcntl.h>
#include <unistd.h>

namespace whatsie::platform {

namespace {

// Signal-handler state must be plain globals: nothing Qt or heap-based is safe to
// touch once a fatal signal has fired.
char s_path[4096] = {};
QString s_lastCrash;

void writeAll(int fd, const char* text)
{
    const ssize_t written = ::write(fd, text, std::strlen(text));
    (void)written; // best-effort; we are already crashing
}

const char* signalName(int sig)
{
    switch (sig) {
    case SIGSEGV:
        return "SIGSEGV (segmentation fault)";
    case SIGABRT:
        return "SIGABRT (abort)";
    case SIGBUS:
        return "SIGBUS (bus error)";
    case SIGILL:
        return "SIGILL (illegal instruction)";
    case SIGFPE:
        return "SIGFPE (floating-point exception)";
    default:
        return "fatal signal";
    }
}

extern "C" void handleFatalSignal(int sig)
{
    if (s_path[0] != '\0') {
        const int fd = ::open(s_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd >= 0) {
            writeAll(fd, "crashed on ");
            writeAll(fd, signalName(sig));
            writeAll(fd, "\n\n");
            std::array<void*, 64> frames{};
            const int count = ::backtrace(frames.data(), static_cast<int>(frames.size()));
            ::backtrace_symbols_fd(frames.data(), count, fd); // async-signal-safe
            ::close(fd);
        }
    }
    // Restore the default action and re-raise so a core dump is still produced.
    ::signal(sig, SIG_DFL);
    ::raise(sig);
}

} // namespace

void installCrashHandler(const QString& crashFilePath)
{
    QFile previous(crashFilePath);
    if (previous.exists()) {
        if (previous.open(QIODevice::ReadOnly)) {
            s_lastCrash = QString::fromUtf8(previous.readAll());
            previous.close();
        }
        previous.remove();
    }

    const QByteArray encoded = crashFilePath.toLocal8Bit();
    std::strncpy(s_path, encoded.constData(), sizeof(s_path) - 1);

    // Warm up libgcc's unwinder so backtrace() in the handler needs no first-call
    // allocation (which would not be async-signal-safe).
    std::array<void*, 4> warm{};
    ::backtrace(warm.data(), static_cast<int>(warm.size()));

    struct sigaction action = {};
    action.sa_handler = handleFatalSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    for (const int sig : {SIGSEGV, SIGABRT, SIGBUS, SIGILL, SIGFPE}) {
        sigaction(sig, &action, nullptr);
    }
}

QString lastCrashReport()
{
    return s_lastCrash;
}

} // namespace whatsie::platform
