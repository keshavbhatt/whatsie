#include "core/render_crash_policy.h"

#include <QTest>

using namespace std::chrono_literals;
using whatsie::core::RenderCrashPolicy;

class TestRenderCrashPolicy : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void retriesWithGrowingDelayThenGivesUp()
    {
        RenderCrashPolicy policy(3, 2min);
        auto d1 = policy.onCrash(0ms);
        QVERIFY(d1.reload);
        QCOMPARE(d1.delay, 1000ms);

        auto d2 = policy.onCrash(5s);
        QVERIFY(d2.reload);
        QCOMPARE(d2.delay, 3000ms);

        auto d3 = policy.onCrash(10s);
        QVERIFY(d3.reload);
        QCOMPARE(d3.delay, 9000ms);

        auto d4 = policy.onCrash(15s);
        QVERIFY(!d4.reload);
        QCOMPARE(policy.crashesInWindow(), 4);
    }

    void oldCrashesFallOutOfWindow()
    {
        RenderCrashPolicy policy(2, 1min);
        QVERIFY(policy.onCrash(0s).reload);
        QVERIFY(policy.onCrash(1s).reload);
        QVERIFY(!policy.onCrash(2s).reload);
        // Two minutes later everything expired: fresh episode.
        auto later = policy.onCrash(3min);
        QVERIFY(later.reload);
        QCOMPARE(later.delay, 1000ms);
        QCOMPARE(policy.crashesInWindow(), 1);
    }

    void successfulLoadResetsEpisode()
    {
        RenderCrashPolicy policy(1, 1h);
        QVERIFY(policy.onCrash(0s).reload);
        policy.onLoadSucceeded();
        QCOMPARE(policy.crashesInWindow(), 0);
        QVERIFY(policy.onCrash(1s).reload);
    }

    void zeroRetriesStillAllowsOne()
    {
        RenderCrashPolicy policy(0, 1h);
        QVERIFY(policy.onCrash(0s).reload);
        QVERIFY(!policy.onCrash(1s).reload);
    }
};

QTEST_GUILESS_MAIN(TestRenderCrashPolicy)
#include "tst_render_crash_policy.moc"
