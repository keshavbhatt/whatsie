#include "core/unread_badge.h"

#include <QColor>
#include <QPainter>
#include <QTest>

using namespace Qt::StringLiterals;
using namespace whatsie::core;

class TestUnreadBadge : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void parsesTitles_data()
    {
        QTest::addColumn<QString>("title");
        QTest::addColumn<int>("expected");
        QTest::newRow("none") << u"WhatsApp"_s << 0;
        QTest::newRow("one") << u"(1) WhatsApp"_s << 1;
        QTest::newRow("many") << u"(42) WhatsApp"_s << 42;
        QTest::newRow("plus") << u"(99+) WhatsApp"_s << 99;
        QTest::newRow("leading space") << u"  (7) WhatsApp"_s << 7;
        QTest::newRow("zero") << u"(0) WhatsApp"_s << 0;
        QTest::newRow("not at start") << u"WhatsApp (3)"_s << 0;
        QTest::newRow("garbage") << u"(abc) WhatsApp"_s << 0;
        QTest::newRow("empty") << QString() << 0;
    }

    void parsesTitles()
    {
        QFETCH(QString, title);
        QFETCH(int, expected);
        QCOMPARE(unreadCountFromTitle(title), expected);
    }

    void zeroCountLeavesImageUntouched()
    {
        const QImage base = solid(64, Qt::blue);
        QCOMPARE(composeUnreadBadge(base, 0), base);
        QCOMPARE(composeUnreadBadge(base, -3), base);
    }

    void positiveCountDrawsBubbleInBottomRight()
    {
        const QImage base = solid(64, Qt::blue);
        const QImage out = composeUnreadBadge(base, 5);
        QCOMPARE(out.size(), base.size());
        // Bubble centre is reddish, top-left corner is still the base colour.
        const QColor centre = out.pixelColor(64 - 10, 64 - 10);
        QVERIFY(centre.red() > 150 && centre.blue() < 100);
        QCOMPARE(out.pixelColor(2, 2), QColor(Qt::blue));
    }

    void largeCountsStillFit()
    {
        const QImage base = solid(32, Qt::green);
        const QImage out = composeUnreadBadge(base, 1234);
        QCOMPARE(out.size(), base.size());
        QVERIFY(out != base);
    }

    void nullImagePassesThrough() { QVERIFY(composeUnreadBadge(QImage(), 3).isNull()); }

    void dimImageDesaturatesAndFades()
    {
        const QImage base = solid(16, QColor(0, 200, 0)); // saturated green
        const QColor before = base.pixelColor(8, 8);
        const QImage dim = dimImage(base, 0.85);
        QCOMPARE(dim.size(), base.size());
        const QColor after = dim.pixelColor(8, 8);
        // Toward grey: red/blue channels rise toward the green's luma.
        QVERIFY(after.red() > before.red());
        QVERIFY(after.green() < before.green());
        // And more transparent than the opaque original.
        QVERIFY(after.alpha() < before.alpha());
    }

    void dimImageZeroAmountIsIdentity()
    {
        const QImage base = solid(8, Qt::red);
        QCOMPARE(dimImage(base, 0.0), base);
    }

    void dimImageNullPassesThrough() { QVERIFY(dimImage(QImage(), 0.5).isNull()); }

    void tintImageRecoloursKeepingAlpha()
    {
        // A half-transparent black square tinted red: RGB becomes red, alpha kept.
        QImage src(4, 4, QImage::Format_ARGB32);
        src.fill(QColor(0, 0, 0, 128));
        const QImage out = tintImage(src, QColor(Qt::red));
        const QColor px = out.pixelColor(1, 1);
        QCOMPARE(px.alpha(), 128);
        QVERIFY(px.red() > 200 && px.green() < 40 && px.blue() < 40);
    }

    void tintImageFullyTransparentStaysTransparent()
    {
        QImage src(4, 4, QImage::Format_ARGB32);
        src.fill(Qt::transparent);
        const QImage out = tintImage(src, QColor(Qt::green));
        QCOMPARE(out.pixelColor(2, 2).alpha(), 0);
    }

    void tintImageNullPassesThrough() { QVERIFY(tintImage(QImage(), Qt::white).isNull()); }

    void monochromeIconIsWhiteWithDarkHalo()
    {
        // A small opaque square glyph -> white centre, dark halo just outside it.
        QImage glyph(64, 64, QImage::Format_ARGB32);
        glyph.fill(Qt::transparent);
        QPainter p(&glyph);
        p.fillRect(QRect(20, 20, 24, 24), Qt::black); // opaque glyph body
        p.end();
        const QImage out = monochromeIcon(glyph);
        QCOMPARE(out.size(), glyph.size());
        QCOMPARE(out.pixelColor(32, 32), QColor(Qt::white)); // fill is white
        // Just outside the body, the halo is dark and semi-opaque.
        const QColor halo = out.pixelColor(19, 32);
        QVERIFY(halo.alpha() > 0);
        QVERIFY(halo.red() < 80 && halo.green() < 80 && halo.blue() < 80);
    }

    void monochromeIconNullPassesThrough() { QVERIFY(monochromeIcon(QImage()).isNull()); }

private:
    static QImage solid(int size, const QColor& color)
    {
        QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
        image.fill(color);
        return image;
    }
};

QTEST_MAIN(TestUnreadBadge)
#include "tst_unread_badge.moc"
