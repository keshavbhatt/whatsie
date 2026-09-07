#include "core/spellcheck.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
using namespace whatsie::core;

class TestSpellcheck : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void localeToDictName_data()
    {
        QTest::addColumn<QString>("locale");
        QTest::addColumn<QString>("dict");
        QTest::newRow("en_US") << u"en_US"_s << u"en-US"_s;
        QTest::newRow("de_DE") << u"de_DE"_s << u"de-DE"_s;
        QTest::newRow("lang expands to region") << u"fr"_s << u"fr-FR"_s;
        QTest::newRow("C") << u"C"_s << u"en-US"_s;
    }

    void localeToDictName()
    {
        QFETCH(QString, locale);
        QFETCH(QString, dict);
        QCOMPARE(dictionaryNameForLocale(QLocale(locale)), dict);
    }

    void emptyLocaleFallsBack() { QCOMPARE(dictionaryNameForLocale(QLocale(u"C"_s)), u"en-US"_s); }

    void findsFirstExistingDictionariesDir()
    {
        QTemporaryDir a;
        QTemporaryDir b;
        QVERIFY(a.isValid() && b.isValid());
        QVERIFY(QDir(b.path()).mkdir(u"qtwebengine_dictionaries"_s));
        const QString found = findDictionariesPath({a.path(), b.path()});
        QCOMPARE(found, QDir(b.path()).filePath(u"qtwebengine_dictionaries"_s));
    }

    void returnsEmptyWhenNoneExist()
    {
        QTemporaryDir a;
        QVERIFY(findDictionariesPath({a.path(), QString()}).isEmpty());
    }

    void listsAvailableBdicNames()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        for (const QString& n : {u"en-US"_s, u"de-DE"_s, u"notes.txt"_s}) {
            QFile f(dir.filePath(n.contains(u'.') ? n : n + u".bdic"_s));
            QVERIFY(f.open(QIODevice::WriteOnly));
        }
        QCOMPARE(availableDictionaries(dir.path()), (QStringList{u"de-DE"_s, u"en-US"_s}));
        QVERIFY(availableDictionaries(QString()).isEmpty());
    }

    void resolvesExactThenSameLanguageElseEmpty()
    {
        const QStringList avail{u"de-DE"_s, u"en-GB"_s, u"en-US"_s, u"fr-FR"_s};
        QCOMPARE(resolveDictionary(u"en-US"_s, avail), u"en-US"_s); // exact
        QCOMPARE(resolveDictionary(u"en-IN"_s, avail), u"en-GB"_s); // same language, first variant
        QCOMPARE(resolveDictionary(u"fr-CA"_s, avail), u"fr-FR"_s); // same language
        QVERIFY(resolveDictionary(u"ru-RU"_s, avail).isEmpty()); // no Russian -> empty, never cross-language
        QVERIFY(resolveDictionary(u"en-US"_s, {}).isEmpty());    // nothing installed
        QCOMPARE(resolveDictionary(u"en"_s, QStringList{u"en"_s, u"en-US"_s}),
                 u"en"_s); // bare language present
    }

    void mergesBundledAndSideloadedDictionaries()
    {
        QTemporaryDir bundled;
        QTemporaryDir user;
        QVERIFY(bundled.isValid() && user.isValid());
        auto touch = [](const QString& path) {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("x");
        };
        touch(bundled.filePath(u"en-US.bdic"_s)); // bundled
        touch(bundled.filePath(u"de-DE.bdic"_s)); // bundled
        touch(user.filePath(u"pl-PL.bdic"_s));    // user sideloaded a new language
        touch(user.filePath(u"en-US.bdic"_s));    // user override of a bundled one

        const QString dir = prepareDictionaryDir(bundled.path(), user.path());
        QCOMPARE(dir, user.path());
        // All languages visible in the one directory QtWebEngine searches.
        QCOMPARE(availableDictionaries(dir), (QStringList{u"de-DE"_s, u"en-US"_s, u"pl-PL"_s}));
        // Bundled linked in; the user's own real files are never clobbered.
        QVERIFY(QFileInfo(user.filePath(u"de-DE.bdic"_s)).isSymLink());
        QVERIFY(!QFileInfo(user.filePath(u"en-US.bdic"_s)).isSymLink());
        QVERIFY(!QFileInfo(user.filePath(u"pl-PL.bdic"_s)).isSymLink());
    }

    void refreshesStaleBundledSymlinks()
    {
        QTemporaryDir bundled;
        QTemporaryDir user;
        QVERIFY(bundled.isValid() && user.isValid());
        // A leftover link to a bundled dict from a previous (gone) install path.
        QVERIFY(QFile::link(u"/nonexistent/old/fr-FR.bdic"_s, user.filePath(u"fr-FR.bdic"_s)));
        QFile f(bundled.filePath(u"it-IT.bdic"_s));
        QVERIFY(f.open(QIODevice::WriteOnly));

        const QString dir = prepareDictionaryDir(bundled.path(), user.path());
        QCOMPARE(availableDictionaries(dir), (QStringList{u"it-IT"_s})); // stale gone, current linked
    }

    void fallsBackToBundledWhenNoWritableDir()
    {
        QTemporaryDir bundled;
        QVERIFY(bundled.isValid());
        QCOMPARE(prepareDictionaryDir(bundled.path(), QString()), bundled.path());
    }
};

QTEST_MAIN(TestSpellcheck)
#include "tst_spellcheck.moc"
