#include "web/user_agent.h"

#include <QTest>

using namespace Qt::StringLiterals;
using whatsie::web::effectiveUserAgent;
using whatsie::web::sanitizeUserAgent;

class TestUserAgent : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void stripsQtWebEngineToken()
    {
        const QString in = u"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
                           "QtWebEngine/6.11.1 Chrome/130.0.6723.192 Safari/537.36"_s;
        const QString out = sanitizeUserAgent(in);
        QVERIFY(!out.contains(u"QtWebEngine"_s));
        QVERIFY(out.contains(u"Chrome/130.0.6723.192"_s));
        QVERIFY(!out.contains(u"  "_s));
    }

    void leavesPlainChromeUntouched()
    {
        const QString in = u"Mozilla/5.0 (X11; Linux x86_64) Chrome/130.0.0.0 Safari/537.36"_s;
        QCOMPARE(sanitizeUserAgent(in), in);
    }

    void overrideWins()
    {
        QCOMPARE(effectiveUserAgent(u"engine QtWebEngine/6.11.1"_s, u"  custom  "_s), u"custom"_s);
    }

    void emptyOverrideFallsBackToSanitizedDefault()
    {
        QCOMPARE(effectiveUserAgent(u"engine QtWebEngine/6.11.1"_s, u"   "_s), u"engine"_s);
    }
};

QTEST_GUILESS_MAIN(TestUserAgent)
#include "tst_user_agent.moc"
