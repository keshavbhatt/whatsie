#include "core/settings/settings.h"
#include "web/proxy.h"

#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
using namespace whatsie::core;
using whatsie::web::toNetworkProxy;

class TestProxy : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void noneIsDirect()
    {
        ProxyConfig c;
        c.mode = ProxyMode::None;
        QCOMPARE(toNetworkProxy(c).type(), QNetworkProxy::NoProxy);
    }

    void systemMapsToDefault()
    {
        ProxyConfig c;
        c.mode = ProxyMode::System;
        QCOMPARE(toNetworkProxy(c).type(), QNetworkProxy::DefaultProxy);
    }

    void manualHttpCarriesHostPortCreds()
    {
        ProxyConfig c;
        c.mode = ProxyMode::Manual;
        c.type = ProxyType::Http;
        c.host = u"proxy.example"_s;
        c.port = 3128;
        c.user = u"alice"_s;
        c.password = u"secret"_s;
        const QNetworkProxy p = toNetworkProxy(c);
        QCOMPARE(p.type(), QNetworkProxy::HttpProxy);
        QCOMPARE(p.hostName(), u"proxy.example"_s);
        QCOMPARE(p.port(), quint16{3128});
        QCOMPARE(p.user(), u"alice"_s);
        QCOMPARE(p.password(), u"secret"_s);
    }

    void manualSocks5Type()
    {
        ProxyConfig c;
        c.mode = ProxyMode::Manual;
        c.type = ProxyType::Socks5;
        c.host = u"s"_s;
        c.port = 1080;
        QCOMPARE(toNetworkProxy(c).type(), QNetworkProxy::Socks5Proxy);
    }

    void passwordNeverPersists()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(u"proxy.ini"_s);
        {
            Settings s(path);
            s.setProxyUser(u"alice"_s);
            s.setProxyPassword(u"hunter2"_s);
            QCOMPARE(s.proxyConfig().password, u"hunter2"_s);
            s.sync();
        }
        Settings reopened(path);
        QCOMPARE(reopened.proxyUser(), u"alice"_s);  // user IS persisted
        QVERIFY(reopened.proxyPassword().isEmpty()); // password is NOT
    }
};

QTEST_MAIN(TestProxy)
#include "tst_proxy.moc"
