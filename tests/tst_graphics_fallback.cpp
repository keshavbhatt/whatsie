#include "core/graphics_fallback.h"

#include <QTest>

using namespace Qt::StringLiterals;
using namespace whatsie::core;

class TestGraphicsFallback : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void detectsRealFailures_data()
    {
        QTest::addColumn<QString>("message");
        QTest::addColumn<bool>("isFailure");
        QTest::newRow("egl") << u"eglInitialize failed"_s << true;
        QTest::newRow("opengl ctx") << u"Failed to create OpenGL context"_s << true;
        QTest::newRow("rhi") << u"Failed to create RHI"_s << true;
        QTest::newRow("backend") << u"Failed to initialize graphics backend for OpenGL"_s << true;
        QTest::newRow("case") << u"FAILED TO CREATE OPENGL CONTEXT"_s << true;
        QTest::newRow("normal gpu log") << u"Using GPU rasterization"_s << false;
        QTest::newRow("unrelated") << u"WebSocket connected"_s << false;
        QTest::newRow("empty") << QString() << false;
    }

    void detectsRealFailures()
    {
        QFETCH(QString, message);
        QFETCH(bool, isFailure);
        QCOMPARE(isGraphicsInitFailure(message), isFailure);
    }

    void detectsGbmVulkanFallback_data()
    {
        QTest::addColumn<QString>("message");
        QTest::addColumn<bool>("isFallback");
        QTest::newRow("nvidia gbm") << u"GBM is not supported with the current configuration. "
                                       "Fallback to Vulkan rendering in Chromium."_s
                                    << true;
        QTest::newRow("case") << u"gbm IS NOT SUPPORTED"_s << true;
        QTest::newRow("normal gpu log") << u"Using GPU rasterization"_s << false;
        QTest::newRow("real failure") << u"eglInitialize failed"_s << false;
        QTest::newRow("empty") << QString() << false;
    }

    void detectsGbmVulkanFallback()
    {
        QFETCH(QString, message);
        QFETCH(bool, isFallback);
        QCOMPARE(isGbmVulkanFallback(message), isFallback);
    }

    void retriesOnlyOnWaylandOnceAfterFailure()
    {
        QCOMPARE(shouldRetryUnderXcb("wayland"_L1, false, true), true);
        QCOMPARE(shouldRetryUnderXcb("wayland"_L1, true, true), false);   // already retried
        QCOMPARE(shouldRetryUnderXcb("wayland"_L1, false, false), false); // no failure seen
        QCOMPARE(shouldRetryUnderXcb("xcb"_L1, false, true), false);      // not Wayland
    }
};

QTEST_MAIN(TestGraphicsFallback)
#include "tst_graphics_fallback.moc"
