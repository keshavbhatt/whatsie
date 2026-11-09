#include "platform/gpu_stderr_watch.h"

#include <QSignalSpy>
#include <QTest>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

using whatsie::platform::GpuStderrWatch;

// Point the real stderr at /dev/null while a watch is installed, so the
// forwarded flood does not swamp the test output. Restored on destruction.
class SilencedStderr
{
public:
    SilencedStderr()
        : m_saved(::dup(STDERR_FILENO))
    {
        const int devNull = ::open("/dev/null", O_WRONLY);
        ::dup2(devNull, STDERR_FILENO);
        ::close(devNull);
    }
    ~SilencedStderr()
    {
        ::dup2(m_saved, STDERR_FILENO);
        ::close(m_saved);
    }
    SilencedStderr(const SilencedStderr&) = delete;
    SilencedStderr& operator=(const SilencedStderr&) = delete;

private:
    int m_saved;
};

class TestGpuStderrWatch : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // Writing far more than a pipe buffer (64 KiB) to stderr from the GUI thread,
    // without returning to the event loop, must not block. It used to: only the
    // GUI thread drained the pipe, so a GPU-process flood during start-up froze
    // the app on its next log line (#387).
    void floodWithoutEventLoopDoesNotBlock()
    {
        const SilencedStderr silenced;
        GpuStderrWatch watch;
        QVERIFY(watch.install());

        const std::string line(1023, 'x');
        for (int i = 0; i < 1024; ++i) { // 1 MiB
            const std::string withNewline = line + '\n';
            QCOMPARE(::write(STDERR_FILENO, withNewline.data(), withNewline.size()),
                     static_cast<ssize_t>(withNewline.size()));
        }
    }

    // The storm is still detected, and reported on the GUI thread.
    void stormIsReported()
    {
        const SilencedStderr silenced;
        GpuStderrWatch watch;
        QSignalSpy spy(&watch, &GpuStderrWatch::gpuContextLostStorm);
        QVERIFY(watch.install());

        const std::string line = "[ERROR:gl_context_egl.cc] eglMakeCurrent failed with error EGL_BAD_DISPLAY\n";
        for (int i = 0; i < 200; ++i) {
            QCOMPARE(::write(STDERR_FILENO, line.data(), line.size()), static_cast<ssize_t>(line.size()));
        }
        QTRY_COMPARE(spy.count(), 1);
    }

    // Tearing the watch down restores the original stderr.
    void destructionRestoresStderr()
    {
        const SilencedStderr silenced;
        struct stat before{};
        QCOMPARE(::fstat(STDERR_FILENO, &before), 0);
        {
            GpuStderrWatch watch;
            QVERIFY(watch.install());
        }
        struct stat after{};
        QCOMPARE(::fstat(STDERR_FILENO, &after), 0);
        QCOMPARE(after.st_ino, before.st_ino);
        QCOMPARE(after.st_dev, before.st_dev);
    }
};

QTEST_GUILESS_MAIN(TestGpuStderrWatch)
#include "tst_gpu_stderr_watch.moc"
