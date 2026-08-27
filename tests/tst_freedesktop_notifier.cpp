// Real D-Bus round trip on a private dbus-daemon: Notify → id mapping →
// ActionInvoked filtering → NotificationClosed, plus RGBA image-data
// marshalling (the W#312/W#328 colour bug and the W#278 "raises on any
// notification click" bug).

#include "platform/linux/dbus_image.h"
#include "platform/linux/freedesktop_notifier.h"

#include <QDBusConnection>
#include <QFile>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
using whatsie::platform::linux_::DBusImage;
using whatsie::platform::linux_::FreedesktopNotifier;

class FakeNotificationServer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")

public Q_SLOTS:
    uint Notify(const QString& appName, uint replacesId, const QString& appIcon, const QString& summary,
                const QString& body, const QStringList& actions, const QVariantMap& hints, int timeout)
    {
        Q_UNUSED(appName)
        Q_UNUSED(replacesId)
        Q_UNUSED(appIcon)
        Q_UNUSED(timeout)
        lastSummary = summary;
        lastBody = body;
        lastActions = actions;
        lastHints = hints;
        return ++counter;
    }
    void CloseNotification(uint id) { closedIds.append(id); }

Q_SIGNALS:
    void ActionInvoked(uint id, const QString& actionKey);
    void NotificationClosed(uint id, uint reason);

public:
    uint counter = 100;
    QString lastSummary;
    QString lastBody;
    QStringList lastActions;
    QVariantMap lastHints;
    QList<uint> closedIds;
};

namespace {
whatsie::core::Notification note(const QString& title)
{
    whatsie::core::Notification n;
    n.title = title;
    return n;
}
} // namespace

class TestFreedesktopNotifier : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        const QString daemon = QStandardPaths::findExecutable(u"dbus-daemon"_s);
        if (daemon.isEmpty()) {
            QSKIP("dbus-daemon not found");
        }
        QVERIFY(m_dir.isValid());
        const QString socketPath = m_dir.filePath(u"bus"_s);
        m_address = u"unix:path=%1"_s.arg(socketPath);
        // Dev builds run tests with the runtime-snap LD_LIBRARY_PATH; the host
        // dbus-daemon must not pick those libraries up.
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.remove(u"LD_LIBRARY_PATH"_s);
        m_daemon.setProcessEnvironment(env);
        m_daemon.setProcessChannelMode(QProcess::MergedChannels);
        m_daemon.start(daemon,
                       {u"--session"_s, u"--nofork"_s, u"--nopidfile"_s, u"--address="_s + m_address});
        QVERIFY(m_daemon.waitForStarted());
        QTRY_VERIFY2_WITH_TIMEOUT(QFile::exists(socketPath),
                                  qPrintable(QString::fromUtf8(m_daemon.readAll())), 5000);

        m_serverBus =
            std::make_unique<QDBusConnection>(QDBusConnection::connectToBus(m_address, u"server"_s));
        m_clientBus =
            std::make_unique<QDBusConnection>(QDBusConnection::connectToBus(m_address, u"client"_s));
        QVERIFY(m_serverBus->isConnected());
        QVERIFY(m_clientBus->isConnected());

        QVERIFY(m_serverBus->registerService(u"org.freedesktop.Notifications"_s));
        QVERIFY(
            m_serverBus->registerObject(u"/org/freedesktop/Notifications"_s, &m_server,
                                        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals));
    }

    void cleanupTestCase()
    {
        m_serverBus.reset();
        m_clientBus.reset();
        QDBusConnection::disconnectFromBus(u"server"_s);
        QDBusConnection::disconnectFromBus(u"client"_s);
        m_daemon.kill();
        m_daemon.waitForFinished(2000);
    }

    void imageDataIsRgbaByteOrder()
    {
        QImage img(2, 1, QImage::Format_ARGB32);
        img.setPixelColor(0, 0, QColor(255, 0, 0, 255)); // red
        img.setPixelColor(1, 0, QColor(0, 0, 255, 128)); // half-transparent blue
        const DBusImage d = whatsie::platform::linux_::toDBusImage(img);
        QCOMPARE(d.width, 2);
        QCOMPARE(d.height, 1);
        QCOMPARE(d.rowstride, 8);
        QCOMPARE(d.channels, 4);
        QVERIFY(d.hasAlpha);
        const auto b = [&](int i) { return static_cast<uchar>(d.data.at(i)); };
        QCOMPARE(b(0), 255); // R
        QCOMPARE(b(1), 0);   // G
        QCOMPARE(b(2), 0);   // B
        QCOMPARE(b(3), 255); // A
        QCOMPARE(b(4), 0);
        QCOMPARE(b(6), 255); // B of second pixel
        QCOMPARE(b(7), 128); // A (non-premultiplied)
        QCOMPARE(whatsie::platform::linux_::fromDBusImage(d).pixelColor(1, 0), QColor(0, 0, 255, 128));
    }

    void notifyRoundTripAndIdFiltering()
    {
        FreedesktopNotifier notifier(*m_clientBus);
        QVERIFY(notifier.isAvailable());
        QSignalSpy activated(&notifier, &FreedesktopNotifier::activated);
        QSignalSpy closed(&notifier, &FreedesktopNotifier::closed);
        QSignalSpy failed(&notifier, &FreedesktopNotifier::failed);

        whatsie::core::Notification n;
        n.title = u"Alice"_s;
        n.body = u"hello"_s;
        n.sound = true;
        n.desktopEntry = u"com.ktechpit.whatsie"_s;
        n.image = QImage(4, 4, QImage::Format_ARGB32);
        n.image.fill(Qt::green);

        notifier.show(42, n);
        QTRY_COMPARE(m_server.lastSummary, u"Alice"_s);
        QCOMPARE(m_server.lastBody, u"hello"_s);
        QVERIFY(m_server.lastActions.contains(u"default"_s));
        QCOMPARE(m_server.lastHints.value(u"category"_s).toString(), u"im.received"_s);
        QCOMPARE(m_server.lastHints.value(u"desktop-entry"_s).toString(), u"com.ktechpit.whatsie"_s);
        QCOMPARE(m_server.lastHints.value(u"sound-name"_s).toString(), u"message-new-instant"_s);
        QVERIFY(m_server.lastHints.contains(u"image-data"_s));
        // Wire signature must match the spec so real daemons (KDE, GNOME) accept it.
        const auto rawImage = m_server.lastHints.value(u"image-data"_s).value<QDBusArgument>();
        QCOMPARE(rawImage.currentSignature(), u"(iiibiiay)"_s);
        QCOMPARE(m_server.lastHints.value(u"urgency"_s).typeId(), QMetaType::UChar);
        const auto received = qdbus_cast<DBusImage>(m_server.lastHints.value(u"image-data"_s));
        QCOMPARE(received.width, 4);
        QCOMPARE(whatsie::platform::linux_::fromDBusImage(received).pixelColor(0, 0), QColor(Qt::green));
        QCOMPARE(failed.count(), 0);

        // Wait for the reply so the id mapping exists, then poke the signals.
        QTest::qWait(100);
        const uint dbusId = m_server.counter;
        Q_EMIT m_server.ActionInvoked(dbusId + 500, u"default"_s); // someone else's
        QTest::qWait(100);
        QCOMPARE(activated.count(), 0);

        Q_EMIT m_server.ActionInvoked(dbusId, u"default"_s);
        QTRY_COMPARE(activated.count(), 1);
        QCOMPARE(activated.first().first().toULongLong(), quint64{42});

        Q_EMIT m_server.NotificationClosed(dbusId, 2);
        QTRY_COMPARE(closed.count(), 1);
        QCOMPARE(closed.first().first().toULongLong(), quint64{42});

        // After close the mapping is gone: a repeat activation is ignored.
        Q_EMIT m_server.ActionInvoked(dbusId, u"default"_s);
        QTest::qWait(100);
        QCOMPARE(activated.count(), 1);
    }

    void closeCallsServer()
    {
        FreedesktopNotifier notifier(*m_clientBus);
        notifier.show(7, note(u"x"_s));
        QTest::qWait(100);
        const uint dbusId = m_server.counter;
        notifier.close(7);
        QTRY_VERIFY(m_server.closedIds.contains(dbusId));
        notifier.close(12345); // unknown: no call, no crash
    }

private:
    QTemporaryDir m_dir;
    QProcess m_daemon;
    QString m_address;
    std::unique_ptr<QDBusConnection> m_serverBus;
    std::unique_ptr<QDBusConnection> m_clientBus;
    FakeNotificationServer m_server;
};

QTEST_MAIN(TestFreedesktopNotifier)
#include "tst_freedesktop_notifier.moc"
