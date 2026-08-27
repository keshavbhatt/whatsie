#include "core/zoom_policy.h"

#include <QTest>

#include <cmath>

using namespace whatsie::core;

class TestZoomPolicy : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void clampsToRange()
    {
        QCOMPARE(clampZoom(0.0), kMinZoom);
        QCOMPARE(clampZoom(10.0), kMaxZoom);
        QCOMPARE(clampZoom(1.3), 1.3);
        QCOMPARE(clampZoom(std::nan("")), kDefaultZoom);
    }

    void stepsAreSymmetricAndRounded()
    {
        QCOMPARE(zoomIn(1.0), 1.1);
        QCOMPARE(zoomOut(1.1), 1.0);
        double z = 1.0;
        for (int i = 0; i < 7; ++i) {
            z = zoomIn(z);
        }
        QCOMPARE(z, 1.7); // no floating drift
        QCOMPARE(zoomIn(kMaxZoom), kMaxZoom);
        QCOMPARE(zoomOut(kMinZoom), kMinZoom);
    }

    void minimumSizeScales()
    {
        QCOMPARE(scaledMinimumSize(QSize(500, 400), 1.0), QSize(500, 400));
        QCOMPARE(scaledMinimumSize(QSize(500, 400), 1.5), QSize(750, 600));
        QCOMPARE(scaledMinimumSize(QSize(500, 400), 0.5), QSize(250, 200));
    }
};

QTEST_GUILESS_MAIN(TestZoomPolicy)
#include "tst_zoom_policy.moc"
