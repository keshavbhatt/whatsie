#include "app/cli_options.h"

#include <QTest>

using namespace Qt::StringLiterals;
using namespace whatsie::app;

class TestCliOptions : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void defaultsWhenNoArguments()
    {
        const CliParseResult r = parseCliOptions({u"whatsie"_s});
        QVERIFY(r.ok());
        QVERIFY(!r.helpRequested);
        QVERIFY(!r.versionRequested);
        QVERIFY(r.options.profile.isEmpty());
        QVERIFY(!r.options.startMinimized);
        QVERIFY(!r.options.logFile.has_value());
        QVERIFY(!r.options.hasCommands());
    }

    void parsesEverything()
    {
        const CliParseResult r = parseCliOptions(
            {u"whatsie"_s, u"--profile"_s, u"work"_s, u"-m"_s, u"--log-file"_s, u"/tmp/x.log"_s,
             u"--new-chat"_s, u"+1 555"_s, u"--settings"_s, u"whatsapp://send?phone=1"_s});
        QVERIFY(r.ok());
        QCOMPARE(r.options.profile, u"work"_s);
        QVERIFY(r.options.startMinimized);
        QCOMPARE(r.options.logFile.value(), u"/tmp/x.log"_s);
        QCOMPARE(r.options.newChat.value(), u"+1 555"_s);
        QVERIFY(r.options.showSettings);
        QCOMPARE(r.options.urls, QStringList{u"whatsapp://send?phone=1"_s});
        QVERIFY(r.options.hasCommands());
    }

    void helpAndVersion()
    {
        const CliParseResult help = parseCliOptions({u"whatsie"_s, u"--help"_s});
        QVERIFY(help.ok());
        QVERIFY(help.helpRequested);
        QVERIFY(help.helpText.contains(u"--profile"_s));

        const CliParseResult version = parseCliOptions({u"whatsie"_s, u"-v"_s});
        QVERIFY(version.ok());
        QVERIFY(version.versionRequested);
    }

    void unknownOptionIsAnError()
    {
        const CliParseResult r = parseCliOptions({u"whatsie"_s, u"--bogus"_s});
        QVERIFY(!r.ok());
        QVERIFY(r.errorText.contains(u"bogus"_s));
    }

    void commandsForwardInOrderAndEndWithRaise()
    {
        CliOptions o;
        o.urls = {u"wa.me/1"_s};
        o.newChat = u"2"_s;
        o.showSettings = true;
        const QList<QJsonObject> cmds = commandsFor(o);
        QCOMPARE(cmds.size(), 4);
        QCOMPARE(cmds[0].value(u"cmd"_s).toString(), u"new-chat"_s);
        QCOMPARE(cmds[0].value(u"url"_s).toString(), u"wa.me/1"_s);
        QCOMPARE(cmds[1].value(u"url"_s).toString(), u"2"_s);
        QCOMPARE(cmds[2].value(u"cmd"_s).toString(), u"settings"_s);
        QCOMPARE(cmds[3].value(u"cmd"_s).toString(), u"raise"_s);
    }

    void quitIsExclusive()
    {
        CliOptions o;
        o.quit = true;
        o.showSettings = true;
        const QList<QJsonObject> cmds = commandsFor(o);
        QCOMPARE(cmds.size(), 1);
        QCOMPARE(cmds[0].value(u"cmd"_s).toString(), u"quit"_s);
    }

    void plainInvocationJustRaises()
    {
        const QList<QJsonObject> cmds = commandsFor(CliOptions{});
        QCOMPARE(cmds.size(), 1);
        QCOMPARE(cmds[0].value(u"cmd"_s).toString(), u"raise"_s);
    }
};

QTEST_GUILESS_MAIN(TestCliOptions)
#include "tst_cli_options.moc"
