#include "core/chromium_flags.h"
#include "core/settings/settings.h"
#include "core/storage_policy.h"

#include <QDir>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
using namespace whatsie::core;

class TestStorageAndFlags : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void safeDeleteOnlyInsideRoots()
    {
        QTemporaryDir root;
        const QString inside = root.filePath(u"profile"_s);
        QVERIFY(QDir().mkpath(inside));
        QVERIFY(QDir().mkpath(root.filePath(u"other"_s)));

        QVERIFY(isSafeToDelete(inside, {root.path()}));
        QVERIFY(!isSafeToDelete(root.path(), {root.path()}));          // the root itself
        QVERIFY(!isSafeToDelete(inside, {}));                          // no roots
        QVERIFY(!isSafeToDelete(inside, {root.filePath(u"other"_s)})); // wrong root
        QVERIFY(!isSafeToDelete(QDir::homePath(), {QDir::homePath()}));
        QVERIFY(!isSafeToDelete(u"/"_s, {u"/"_s}));
        QVERIFY(!isSafeToDelete(root.filePath(u"missing"_s), {root.path()}));
        QVERIFY(!isSafeToDelete(QString(), {root.path()}));
        // A file is not a directory.
        {
            QFile f(root.filePath(u"file"_s));
            QVERIFY(f.open(QIODevice::WriteOnly));
        }
        QVERIFY(!isSafeToDelete(root.filePath(u"file"_s), {root.path()}));
        // Prefix trickery: "/tmp/rootX" is not inside "/tmp/root".
        const QString sibling = root.path() + u"X"_s;
        QVERIFY(QDir().mkpath(sibling));
        QVERIFY(!isSafeToDelete(sibling, {root.path()}));
        QDir(sibling).removeRecursively();
    }

    void removeRefusesOutsideAndDeletesInside()
    {
        QTemporaryDir root;
        const QString inside = root.filePath(u"cache"_s);
        QVERIFY(QDir().mkpath(inside + u"/sub"_s));
        {
            QFile f(inside + u"/sub/a.bin"_s);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(QByteArray(2048, 'x'));
        }
        QCOMPARE(directorySize(inside), 2048);
        QVERIFY(!removeDirectorySafely(root.path(), {root.path()}));
        QVERIFY(QDir(inside).exists());
        QVERIFY(removeDirectorySafely(inside, {root.path()}));
        QVERIFY(!QDir(inside).exists());
    }

    void gpuAutoFallbackChoosesSoftware()
    {
        // useSoftwareGpu: setting wins for On/Off; Auto follows the fallback flag.
        QVERIFY(!useSoftwareGpu(HardwareAcceleration::On, true));  // On always keeps GPU
        QVERIFY(useSoftwareGpu(HardwareAcceleration::Off, false)); // Off always software
        QVERIFY(!useSoftwareGpu(HardwareAcceleration::Auto, false));
        QVERIFY(useSoftwareGpu(HardwareAcceleration::Auto, true)); // auto-disabled -> software
        // Auto + auto-disabled produces the SwiftShader software path.
        const QStringList fallback = chromiumFlags(HardwareAcceleration::Auto, true);
        QVERIFY(fallback.contains(u"--use-angle=swiftshader"_s));
        QVERIFY(fallback.contains(u"--enable-unsafe-swiftshader"_s));
        // Auto + not-disabled keeps the GPU on (no software flags).
        QVERIFY(!chromiumFlags(HardwareAcceleration::Auto, false).contains(u"--use-angle=swiftshader"_s));
    }

    void settingChangeResetsGpuFallback()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        Settings settings(dir.filePath(u"gpu.ini"_s));
        settings.setGpuAutoDisabled(true);
        settings.setGpuProbeStrikes(2);
        // Choosing a specific acceleration re-arms a fresh GPU trial.
        settings.setHardwareAcceleration(HardwareAcceleration::On);
        QVERIFY(!settings.gpuAutoDisabled());
        QCOMPARE(settings.gpuProbeStrikes(), 0);
    }

    void flagsFollowAccelerationSetting()
    {
        const QStringList autoFlags = chromiumFlags(HardwareAcceleration::Auto);
        QVERIFY(!autoFlags.contains(u"--disable-gpu"_s));
        QVERIFY(!autoFlags.contains(u"--ignore-gpu-blocklist"_s));
        QVERIFY(autoFlags.contains(u"--disable-extensions"_s));
        QVERIFY(chromiumFlags(HardwareAcceleration::On).contains(u"--ignore-gpu-blocklist"_s));
        // GPU-off uses ANGLE→SwiftShader (software) so WhatsApp calls keep a
        // WebGL context rather than a blank video (ADR-032).
        const QStringList off = chromiumFlags(HardwareAcceleration::Off);
        QVERIFY(off.contains(u"--use-angle=swiftshader"_s));
        QVERIFY(off.contains(u"--enable-unsafe-swiftshader"_s));
        QVERIFY(!chromiumFlags(HardwareAcceleration::On).contains(u"--use-angle=swiftshader"_s));
        QVERIFY(autoFlags.join(u' ').contains(u"--enable-features=SharedArrayBuffer"_s));
#ifdef Q_OS_LINUX
        QVERIFY(autoFlags.join(u' ').contains(u"WebRTCPipeWireCapturer"_s));
        // Hardware video decode is off on Linux (ADR-032): its dmabuf frames
        // crash the compositor GL context on many Mesa/Wayland/snap stacks.
        QVERIFY(autoFlags.contains(u"--disable-accelerated-video-decode"_s));
        QVERIFY(chromiumFlags(HardwareAcceleration::On).contains(u"--disable-accelerated-video-decode"_s));
#endif
    }

    void userFlagsAreKeptAndDeduplicated()
    {
        const QString merged =
            mergeChromiumFlags(u"--foo=1  --disable-gpu"_s, {u"--disable-gpu"_s, u"--bar"_s});
        QCOMPARE(merged, u"--foo=1 --disable-gpu --bar"_s);
        QCOMPARE(mergeChromiumFlags(QString(), {u"--x"_s}), u"--x"_s);
        // The snap sets QTWEBENGINE_CHROMIUM_FLAGS=--no-sandbox; it must survive the merge.
        QVERIFY(mergeChromiumFlags(u"--no-sandbox"_s, chromiumFlags(HardwareAcceleration::Auto))
                    .contains(u"--no-sandbox"_s));
        // Feature lists are merged, not overridden (Chromium keeps only the last switch).
        QCOMPARE(mergeChromiumFlags(u"--enable-features=A,B --disable-features=X"_s,
                                    {u"--enable-features=B,C"_s, u"--y"_s}),
                 u"--y --enable-features=A,B,C --disable-features=X"_s);
    }

    void downloadDirectorySettingFallsBack()
    {
        QTemporaryDir dir;
        Settings settings(dir.filePath(u"s.ini"_s));
        QVERIFY(QDir(settings.downloadDirectory()).isAbsolute());
        settings.setDownloadDirectory(u"relative/path"_s);
        QVERIFY(QDir(settings.downloadDirectory()).isAbsolute()); // rejected → fallback
        settings.setDownloadDirectory(u"/tmp/../tmp/dl/"_s);
        QCOMPARE(settings.downloadDirectory(), u"/tmp/dl"_s);
        QCOMPARE(settings.hardwareAcceleration(), HardwareAcceleration::Auto);
        settings.setHardwareAcceleration(HardwareAcceleration::Off);
        QCOMPARE(settings.hardwareAcceleration(), HardwareAcceleration::Off);
        QVERIFY(!settings.muted());
        settings.setMuted(true);
        QVERIFY(settings.muted());
    }
};

QTEST_GUILESS_MAIN(TestStorageAndFlags)
#include "tst_storage_and_flags.moc"
