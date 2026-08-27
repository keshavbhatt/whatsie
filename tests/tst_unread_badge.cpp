#include "core/unread_badge.h"

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
