#include "app/single_instance.h"

#include <QRandomGenerator>
#include <QSignalSpy>
#include <QTest>

using namespace Qt::StringLiterals;
using whatsie::app::SingleInstance;

class TestSingleInstance : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init()
    {
        // Unique key per test so parallel ctest runs never collide.
        m_key = u"whatsie-test-%1"_s.arg(QRandomGenerator::global()->generate());
    }

    void firstIsPrimarySecondIsNot()
    {
        SingleInstance primary(m_key);
        QVERIFY(primary.isPrimary());
        SingleInstance secondary(m_key);
        QVERIFY(!secondary.isPrimary());
    }

    void commandsReachThePrimary()
    {
        SingleInstance primary(m_key);
        QSignalSpy spy(&primary, &SingleInstance::commandReceived);
        SingleInstance secondary(m_key);

        QVERIFY(secondary.sendToPrimary({{u"cmd"_s, u"raise"_s}}));
        QVERIFY(secondary.sendToPrimary({{u"cmd"_s, u"new-chat"_s}, {u"url"_s, u"wa.me/1"_s}}));
        QVERIFY(spy.wait(2000));
        if (spy.count() < 2) {
            QVERIFY(spy.wait(2000));
        }
        QCOMPARE(spy.count(), 2);
        const QJsonObject second = spy.at(1).first().toJsonObject();
        QCOMPARE(second.value(u"cmd"_s).toString(), u"new-chat"_s);
        QCOMPARE(second.value(u"url"_s).toString(), u"wa.me/1"_s);
    }

    void primaryRefusesToSend()
    {
        SingleInstance primary(m_key);
        QVERIFY(!primary.sendToPrimary({{u"cmd"_s, u"raise"_s}}));
    }

    void keyIsReleasedOnDestruction()
    {
        {
            SingleInstance primary(m_key);
            QVERIFY(primary.isPrimary());
        }
        SingleInstance next(m_key);
        QVERIFY(next.isPrimary());
    }

    void keyDependsOnProfile()
    {
        QVERIFY(whatsie::app::instanceKeyFor(QString()).contains(u"default"_s));
        QVERIFY(whatsie::app::instanceKeyFor(u"work"_s).contains(u"work"_s));
        QVERIFY(whatsie::app::instanceKeyFor(u"work"_s) != whatsie::app::instanceKeyFor(u"home"_s));
    }

private:
    QString m_key;
};

QTEST_GUILESS_MAIN(TestSingleInstance)
#include "tst_single_instance.moc"
