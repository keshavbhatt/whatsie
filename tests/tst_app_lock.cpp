#include "core/app_lock.h"

#include <QTest>

using namespace Qt::StringLiterals;
using namespace whatsie::core;
using namespace std::chrono_literals;

class TestAppLock : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void roundTripVerifies()
    {
        const PasscodeRecord rec = makePasscode(u"correct horse"_s, 20000);
        QVERIFY(rec.isValid());
        QVERIFY(verifyPasscode(u"correct horse"_s, rec));
        QVERIFY(!verifyPasscode(u"wrong"_s, rec));
        QVERIFY(!verifyPasscode(QString(), rec));
    }

    void saltMakesHashesUnique()
    {
        const PasscodeRecord a = makePasscode(u"1234"_s, 20000);
        const PasscodeRecord b = makePasscode(u"1234"_s, 20000);
        QVERIFY(a.salt != b.salt);
        QVERIFY(a.hash != b.hash); // same passcode, different salt -> different hash
        QVERIFY(verifyPasscode(u"1234"_s, a));
        QVERIFY(verifyPasscode(u"1234"_s, b));
    }

    void deterministicWithFixedSalt()
    {
        const QByteArray salt = QByteArrayLiteral("0123456789abcdef");
        const QByteArray h1 = derivePasscodeHash(u"pw"_s, salt, 15000);
        const QByteArray h2 = derivePasscodeHash(u"pw"_s, salt, 15000);
        QCOMPARE(h1, h2);
        QVERIFY(!h1.isEmpty());
    }

    void invalidRecordNeverVerifies() { QVERIFY(!verifyPasscode(u"x"_s, PasscodeRecord{})); }

    void throttleEscalatesAndCaps()
    {
        QCOMPARE(lockoutDuration(0), 0ms);
        QCOMPARE(lockoutDuration(3), 0ms);
        QCOMPARE(lockoutDuration(4), 5000ms);
        QVERIFY(lockoutDuration(6) > lockoutDuration(4));
        QCOMPARE(lockoutDuration(100), lockoutDuration(9)); // capped
        QCOMPARE(lockoutDuration(100), 300000ms);
    }
};

QTEST_MAIN(TestAppLock)
#include "tst_app_lock.moc"
