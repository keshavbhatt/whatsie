#include "platform/launcher_badge.h"

#include <QTest>

using namespace Qt::StringLiterals;
using namespace whatsie::platform;

class TestLauncherBadge : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void buildsAppUri()
    {
        QCOMPARE(launcherEntryUri(u"com.ktechpit.whatsie"_s),
                 u"application://com.ktechpit.whatsie.desktop"_s);
        // Snap-exported id (issue #357) round-trips the same way.
        QCOMPARE(launcherEntryUri(u"whatsie_whatsie"_s),
                 u"application://whatsie_whatsie.desktop"_s);
    }

    void emptyIdGivesEmptyUri() { QVERIFY(launcherEntryUri(QString()).isEmpty()); }
};

QTEST_MAIN(TestLauncherBadge)
#include "tst_launcher_badge.moc"
