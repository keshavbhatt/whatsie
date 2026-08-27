#include "core/notifications/identicon.h"

#include <QTest>

using namespace Qt::StringLiterals;
using namespace whatsie::core;

class TestIdenticon : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void colourIsStableAndCaseInsensitive()
    {
        QCOMPARE(identiconColor(u"Alice Smith"_s), identiconColor(u"Alice Smith"_s));
        QCOMPARE(identiconColor(u"alice smith"_s), identiconColor(u"ALICE SMITH"_s));
        QVERIFY(identiconColor(u"Alice"_s) != identiconColor(u"Bob"_s));
    }

    void colourIsPinnedAcrossBuilds()
    {
        // Guards the hash function: a change here would recolour every contact.
        QCOMPARE(identiconColor(u"Whatsie"_s).hslHue(), 54);
    }

    void initials()
    {
        QCOMPARE(identiconInitials(u"Alice Smith"_s), u"AS"_s);
        QCOMPARE(identiconInitials(u"  bob  "_s), u"B"_s);
        QCOMPARE(identiconInitials(u"Anna Maria Lopez"_s), u"AM"_s);
        QCOMPARE(identiconInitials(u"+49 170"_s), u"4"_s + u"1"_s);
        QCOMPARE(identiconInitials(u"!!!"_s), u"?"_s);
        QCOMPARE(identiconInitials(QString()), u"?"_s);
    }

    void rendersCircleOfRequestedSize()
    {
        const QImage img = identicon(u"Alice"_s, 64);
        QCOMPARE(img.size(), QSize(64, 64));
        QCOMPARE(img.pixelColor(1, 1).alpha(), 0);    // corner outside the circle
        QVERIFY(img.pixelColor(32, 8).alpha() > 200); // inside the circle
    }
};

QTEST_MAIN(TestIdenticon)
#include "tst_identicon.moc"
