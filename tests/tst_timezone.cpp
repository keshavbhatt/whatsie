#include "core/timezone.h"

#include <QByteArray>
#include <QTest>
#include <QTimeZone>

using namespace Qt::StringLiterals;
using namespace whatsie::core;

class TestTimeZone : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void explicitOverrideWins()
    {
        const TimeZoneChoice choice =
            resolveTimeZone(u"Europe/London"_s, u"America/Los_Angeles"_s, u"UTC"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("Europe/London"));
        QCOMPARE(choice.ianaId, u"Europe/London"_s);
        QCOMPARE(choice.source, u"setting"_s);
        QVERIFY(choice.warning.isEmpty());
    }

    void validEnvIsKept()
    {
        const TimeZoneChoice choice = resolveTimeZone(QString(), u"Europe/London"_s, u"UTC"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("Europe/London"));
        QCOMPARE(choice.source, u"env"_s);
        QVERIFY(choice.warning.isEmpty());
    }

    void colonPrefixedIanaIsNormalized()
    {
        // TZ=:Europe/London is a POSIX file form; ICU can misread the colon.
        const TimeZoneChoice choice = resolveTimeZone(QString(), u":Europe/London"_s, u"UTC"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("Europe/London"));
        QCOMPARE(choice.source, u"env-normalized"_s);
        QVERIFY(choice.warning.isEmpty());
    }

    void colonLocaltimePathFallsBackToSystem()
    {
        // The classic broken value: TZ=:/etc/localtime.
        const TimeZoneChoice choice = resolveTimeZone(QString(), u":/etc/localtime"_s, u"UTC"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("UTC"));
        QCOMPARE(choice.source, u"system"_s);
        QVERIFY(!choice.warning.isEmpty());
    }

    void absolutePathFallsBackToSystem()
    {
        const TimeZoneChoice choice = resolveTimeZone(QString(), u"/etc/localtime"_s, u"UTC"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("UTC"));
        QCOMPARE(choice.source, u"system"_s);
        QVERIFY(!choice.warning.isEmpty());
    }

    void invalidIanaFallsBackToSystem()
    {
        const TimeZoneChoice choice = resolveTimeZone(QString(), u"Europe/Nowhere"_s, u"UTC"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("UTC"));
        QCOMPARE(choice.source, u"system"_s);
        QVERIFY(!choice.warning.isEmpty());
    }

    void emptyEnvUsesSystem()
    {
        const TimeZoneChoice choice = resolveTimeZone(QString(), QString(), u"UTC"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("UTC"));
        QCOMPARE(choice.source, u"system"_s);
        QVERIFY(choice.warning.isEmpty());
    }

    void invalidOverrideIsIgnored()
    {
        const TimeZoneChoice choice =
            resolveTimeZone(u"Not/AZone"_s, u"Europe/London"_s, u"UTC"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("Europe/London"));
        QCOMPARE(choice.source, u"env"_s);
        QVERIFY(choice.warning.contains(u"setting"_s));
    }

    void overrideIsTrimmed()
    {
        const TimeZoneChoice choice =
            resolveTimeZone(u"  Europe/London  "_s, QString(), u"UTC"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("Europe/London"));
        QCOMPARE(choice.source, u"setting"_s);
    }

    void posixStringIsKept()
    {
        // POSIX TZ strings are a deliberate expert choice; do not rewrite them.
        const TimeZoneChoice choice = resolveTimeZone(QString(), u"EST5EDT"_s, u"UTC"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("EST5EDT"));
        QCOMPARE(choice.source, u"env"_s);
    }

    void utcIsKept()
    {
        const TimeZoneChoice choice = resolveTimeZone(QString(), u"UTC"_s, u"Europe/London"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("UTC"));
        QCOMPARE(choice.source, u"env"_s);
    }

    void nothingUsableLeavesTzAlone()
    {
        const TimeZoneChoice choice = resolveTimeZone(QString(), QString(), QString());
        QVERIFY(choice.tzValue.isEmpty());
        QCOMPARE(choice.source, u"none"_s);
        QVERIFY(!choice.warning.isEmpty());
    }

    void edmontonIsAValidZoneOnThisHost()
    {
        // Guards the issue reporter's zone. A host without that tzdata entry is
        // not a bug in the resolver.
        if (!QTimeZone::isTimeZoneIdAvailable(QByteArrayLiteral("America/Edmonton"))) {
            QSKIP("America/Edmonton is missing from this system's tzdata");
        }
        const TimeZoneChoice choice = resolveTimeZone(QString(), QString(), u"America/Edmonton"_s);
        QCOMPARE(choice.tzValue, QByteArrayLiteral("America/Edmonton"));
        QCOMPARE(choice.source, u"system"_s);
    }
};

QTEST_GUILESS_MAIN(TestTimeZone)
#include "tst_timezone.moc"
