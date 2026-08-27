#include "core/spellcheck.h"

#include <QDir>
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
};

QTEST_MAIN(TestSpellcheck)
#include "tst_spellcheck.moc"
