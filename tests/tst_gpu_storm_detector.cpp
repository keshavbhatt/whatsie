#include "core/gpu_storm_detector.h"

#include <QTest>

using namespace Qt::StringLiterals;
using whatsie::core::GpuStormDetector;

class TestGpuStormDetector : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void matchesContextLossLines()
    {
        QVERIFY(GpuStormDetector::isContextLossLine(
            u"eglMakeCurrent failed with error EGL_BAD_DISPLAY"_s));
        QVERIFY(GpuStormDetector::isContextLossLine(
            u"Failed to make current since context is marked as lost"_s));
        QVERIFY(GpuStormDetector::isContextLossLine(u"GpuChannel: Failed to create SharedImageStub"_s));
        QVERIFY(!GpuStormDetector::isContextLossLine(u"just a normal log line"_s));
    }

    void firesOnceOnSustainedStorm()
    {
        GpuStormDetector d;
        bool fired = false;
        for (int i = 0; i < GpuStormDetector::kThreshold + 5; ++i) {
            fired = d.observe(1000 + i, u"...EGL_BAD_DISPLAY"_s) || fired; // ~1ms apart
        }
        QVERIFY(fired);
        QVERIFY(d.fired());
        QVERIFY(!d.observe(5000, u"...EGL_BAD_DISPLAY"_s)); // never fires twice
    }

    void transientBurstDoesNotFire()
    {
        GpuStormDetector d;
        for (int i = 0; i < 13; ++i) { // a normal video blip, not a storm
            QVERIFY(!d.observe(1000 + i * 5, u"EGL_BAD_DISPLAY"_s));
        }
        QVERIFY(!d.fired());
    }

    void hitsSpreadBeyondTheWindowDoNotAccumulate()
    {
        GpuStormDetector d;
        // 100 ms apart: at most ~31 within the 3 s window, below the threshold.
        for (int i = 0; i < GpuStormDetector::kThreshold * 2; ++i) {
            QVERIFY(!d.observe(static_cast<qint64>(i) * 100, u"EGL_BAD_DISPLAY"_s));
        }
        QVERIFY(!d.fired());
    }

    void nonMatchingLinesIgnored()
    {
        GpuStormDetector d;
        for (int i = 0; i < 200; ++i) {
            QVERIFY(!d.observe(1000 + i, u"harmless chatter"_s));
        }
        QVERIFY(!d.fired());
    }
};

QTEST_APPLESS_MAIN(TestGpuStormDetector)
#include "tst_gpu_storm_detector.moc"
