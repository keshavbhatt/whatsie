#include "core/navigation_policy.h"

#include <QTest>

using namespace Qt::StringLiterals;
using namespace whatsie::core;

class TestNavigationPolicy : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void whatsAppWebDetection()
    {
        QVERIFY(isWhatsAppWebUrl(QUrl(u"https://web.whatsapp.com/"_s)));
        QVERIFY(isWhatsAppWebUrl(QUrl(u"https://WEB.whatsapp.com/send?phone=1"_s)));
        QVERIFY(!isWhatsAppWebUrl(QUrl(u"https://whatsapp.com/"_s)));
        QVERIFY(!isWhatsAppWebUrl(QUrl(u"https://example.com/"_s)));
        QVERIFY(!isWhatsAppWebUrl(QUrl(u"whatsapp://send?phone=1"_s)));
    }

    void externalLinks()
    {
        QVERIFY(shouldOpenExternally(QUrl(u"https://example.com/x"_s)));
        QVERIFY(shouldOpenExternally(QUrl(u"http://static.whatsapp.net/x"_s)));
        QVERIFY(!shouldOpenExternally(QUrl(u"https://web.whatsapp.com/x"_s)));
        QVERIFY(!shouldOpenExternally(QUrl(u"blob:https://web.whatsapp.com/abc"_s)));
        QVERIFY(!shouldOpenExternally(QUrl(u"about:blank"_s)));
    }

    void normalizesPhones()
    {
        QCOMPARE(normalizePhone(u"+49 (170) 123-4567"_s), u"491701234567"_s);
        QCOMPARE(normalizePhone(u"abc"_s), QString());
    }

    void parsesChatLinks_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<bool>("ok");
        QTest::addColumn<QString>("phone");
        QTest::addColumn<QString>("text");

        QTest::newRow("scheme") << u"whatsapp://send?phone=%2B491701234567&text=Hi%20there"_s << true
                                << u"491701234567"_s << u"Hi there"_s;
        QTest::newRow("scheme slash")
            << u"whatsapp://send/?phone=15551234"_s << true << u"15551234"_s << QString();
        QTest::newRow("wa.me") << u"https://wa.me/15551234?text=hello"_s << true << u"15551234"_s
                               << u"hello"_s;
        QTest::newRow("api") << u"https://api.whatsapp.com/send?phone=15551234"_s << true << u"15551234"_s
                             << QString();
        QTest::newRow("web") << u"https://web.whatsapp.com/send?phone=15551234&text=x"_s << true
                             << u"15551234"_s << u"x"_s;
        QTest::newRow("plain phone") << u"+1 555 1234"_s << true << u"15551234"_s << QString();
        QTest::newRow("no phone") << u"whatsapp://send?text=hi"_s << false << QString() << QString();
        QTest::newRow("other scheme")
            << u"https://example.com/send?phone=1"_s << false << QString() << QString();
        QTest::newRow("garbage") << u"hello world"_s << false << QString() << QString();
        QTest::newRow("empty") << QString() << false << QString() << QString();
    }

    void parsesChatLinks()
    {
        QFETCH(QString, input);
        QFETCH(bool, ok);
        QFETCH(QString, phone);
        QFETCH(QString, text);

        const auto result = parseChatLink(input);
        QCOMPARE(result.has_value(), ok);
        if (ok) {
            QCOMPARE(result->phone, phone);
            QCOMPARE(result->text, text);
        }
    }

    void buildsNewChatUrl()
    {
        const QUrl url = newChatUrl({.phone = u"+49 170"_s, .text = u"Hi & bye"_s});
        QCOMPARE(url.host(), u"web.whatsapp.com"_s);
        QCOMPARE(url.path(), u"/send"_s);
        QVERIFY(url.query(QUrl::FullyEncoded).contains(u"phone=49170"_s));
        QVERIFY(url.query(QUrl::FullyEncoded).contains(u"text=Hi%20%26%20bye"_s));

        const QUrl noText = newChatUrl({.phone = u"1"_s, .text = {}});
        QVERIFY(!noText.query().contains(u"text"_s));
    }

    void roundTripThroughOwnUrl()
    {
        const NewChatRequest request{.phone = u"15551234"_s, .text = u"héllo wörld"_s};
        const auto parsed = parseChatLink(newChatUrl(request));
        QVERIFY(parsed.has_value());
        QCOMPARE(*parsed, request);
    }
};

QTEST_GUILESS_MAIN(TestNavigationPolicy)
#include "tst_navigation_policy.moc"
