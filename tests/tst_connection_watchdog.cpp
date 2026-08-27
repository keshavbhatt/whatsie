#include "core/connection_watchdog_policy.h"

#include <QTest>

using namespace std::chrono_literals;
using whatsie::core::ConnectionWatchdogPolicy;

class TestConnectionWatchdog : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void noReloadWhileConnected()
    {
        ConnectionWatchdogPolicy p(20s, 3, 15s);
        QVERIFY(p.isConnected());
        QVERIFY(!p.shouldReload(0ms));
        QVERIFY(!p.shouldReload(1h));
    }

    void reloadsAfterGraceThenRespectsCooldownAndCap()
    {
        ConnectionWatchdogPolicy p(20s, 2, 15s);
        p.setConnected(false, 0ms);
        QVERIFY(!p.shouldReload(10s)); // within grace
        QVERIFY(p.shouldReload(20s));  // grace elapsed
        p.noteReload(20s);
        QVERIFY(!p.shouldReload(25s)); // cooldown + new grace
        QVERIFY(!p.shouldReload(35s)); // still within grace after reload
        QVERIFY(p.shouldReload(40s));  // 20s after the reload
        p.noteReload(40s);
        QCOMPARE(p.reloadsThisEpisode(), 2);
        QVERIFY(!p.shouldReload(70s)); // cap reached
    }

    void reconnectEndsEpisode()
    {
        ConnectionWatchdogPolicy p(20s, 2, 15s);
        p.setConnected(false, 0ms);
        p.noteReload(20s);
        p.noteReload(40s);
        QVERIFY(!p.shouldReload(70s)); // capped
        p.setConnected(true, 80s);     // reconnected
        QVERIFY(p.isConnected());
        p.setConnected(false, 100s); // fresh episode
        QCOMPARE(p.reloadsThisEpisode(), 0);
        QVERIFY(p.shouldReload(120s));
    }

    void networkReturnGrantsAnotherAttemptAfterCap()
    {
        ConnectionWatchdogPolicy p(20s, 1, 15s);
        p.setConnected(false, 0ms);
        QVERIFY(p.shouldReload(20s));
        p.noteReload(20s);
        QVERIFY(!p.shouldReload(60s)); // cap = 1 reached
        p.networkReturned(100s);
        QVERIFY(p.shouldReload(100s)); // fresh chance on network return
    }

    void networkReturnIgnoredWhenConnected()
    {
        ConnectionWatchdogPolicy p;
        p.networkReturned(10s);
        QVERIFY(!p.shouldReload(1h));
    }
};

QTEST_GUILESS_MAIN(TestConnectionWatchdog)
#include "tst_connection_watchdog.moc"
