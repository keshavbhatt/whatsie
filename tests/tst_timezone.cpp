#include "core/timezone.h"

#include <QTest>

#include <limits>

using namespace Qt::StringLiterals;
using namespace whatsie::core;

class TestTimeZone : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void formatsPositiveOffsets()
    {
        QCOMPARE(formatUtcOffset(5 * 60 * 60 + 30 * 60), u"+05:30"_s);
        QCOMPARE(formatUtcOffset(60 * 60), u"+01:00"_s);
        QCOMPARE(formatUtcOffset(0), u"+00:00"_s);
    }

    void formatsNegativeOffsets()
    {
        QCOMPARE(formatUtcOffset(-7 * 60 * 60), u"-07:00"_s);
        QCOMPARE(formatUtcOffset(-3 * 60 * 60 - 30 * 60), u"-03:30"_s);
    }

    void unavailableOffsetsAreGraceful()
    {
        QCOMPARE(formatUtcOffset(std::nullopt), u"unavailable"_s);
        QCOMPARE(formatUtcOffset(1), u"unavailable"_s);
        QCOMPARE(formatUtcOffset(25 * 60 * 60), u"unavailable"_s);
        QCOMPARE(formatUtcOffset(std::numeric_limits<int>::min()), u"unavailable"_s);
    }
};

QTEST_GUILESS_MAIN(TestTimeZone)
#include "tst_timezone.moc"
