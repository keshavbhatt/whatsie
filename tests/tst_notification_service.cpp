#include "core/notifications/dnd_controller.h"
#include "core/notifications/notification_service.h"
#include "core/settings/settings.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;
using namespace whatsie::core;

class FakeNotifier : public INotifier
{
    Q_OBJECT
public:
    using INotifier::INotifier;
    QString name() const override { return u"fake"_s; }
    bool isAvailable() const override { return available; }
    void show(quint64 id, const Notification& n) override
    {
        shown.append({id, n});
        if (failNext) {
            failNext = false;
            Q_EMIT failed(id, u"boom"_s);
        }
    }
    void close(quint64 id) override { closedIds.append(id); }

    bool available = true;
    bool failNext = false;
    QList<QPair<quint64, Notification>> shown;
    QList<quint64> closedIds;
};

namespace {
Notification note(const QString& title)
{
    Notification n;
    n.title = title;
    return n;
}
} // namespace

class TestNotificationService : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init()
    {
        m_dir = std::make_unique<QTemporaryDir>();
        m_settings = std::make_unique<Settings>(m_dir->filePath(u"s.ini"_s));
        m_dnd = std::make_unique<DndController>();
        m_primary = std::make_unique<FakeNotifier>();
        m_fallback = std::make_unique<FakeNotifier>();
        m_service =
            std::make_unique<NotificationService>(*m_settings, *m_dnd, m_primary.get(), m_fallback.get());
    }

    void fillsDefaultsAndAssignsIds()
    {
        m_settings->setNotificationTimeoutSec(7);
        m_settings->setNotificationSound(false);
        Notification n;
        n.title = u"Alice"_s;
        n.body = u"hi"_s;
        const quint64 id = m_service->notify(n);
        QVERIFY(id != 0);
        QCOMPARE(m_primary->shown.size(), 1);
        const Notification& sent = m_primary->shown.first().second;
        QVERIFY(!sent.image.isNull()); // identicon substituted
        QCOMPARE(sent.timeoutMs, 7000);
        QCOMPARE(sent.sound, false);
        QCOMPARE(m_service->activeCount(), 1);

        const quint64 next = m_service->notify(n);
        QVERIFY(next != id);
    }

    void keepsProvidedImage()
    {
        QImage avatar(8, 8, QImage::Format_ARGB32);
        avatar.fill(Qt::red);
        Notification n;
        n.title = u"Bob"_s;
        n.image = avatar;
        m_service->notify(n);
        QCOMPARE(m_primary->shown.first().second.image, avatar);
    }

    void suppressedWhenDisabledOrDnd()
    {
        m_settings->setNotificationsEnabled(false);
        QCOMPARE(m_service->notify(note(u"x"_s)), quint64{0});
        m_settings->setNotificationsEnabled(true);

        m_dnd->enableFor(30min);
        QVERIFY(m_service->isSuppressed());
        QCOMPARE(m_service->notify(note(u"x"_s)), quint64{0});
        m_dnd->disable();
        QVERIFY(m_service->notify(note(u"x"_s)) != 0);
        QCOMPARE(m_primary->shown.size(), 1);
    }

    void activationOnlyForKnownIds()
    {
        QSignalSpy spy(m_service.get(), &NotificationService::activated);
        const quint64 id = m_service->notify(note(u"x"_s));
        Q_EMIT m_primary->activated(999); // foreign id must be ignored
        QCOMPARE(spy.count(), 0);
        Q_EMIT m_primary->activated(id);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toULongLong(), id);
    }

    void closeForwardsToOwnerAndEmitsOnce()
    {
        QSignalSpy spy(m_service.get(), &NotificationService::closed);
        const quint64 id = m_service->notify(note(u"x"_s));
        m_service->close(id);
        QCOMPARE(m_primary->closedIds, QList<quint64>{id});
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_service->activeCount(), 0);
        Q_EMIT m_primary->closed(id); // late backend echo: no second signal
        QCOMPARE(spy.count(), 1);
    }

    void fallsBackWhenPrimaryFails()
    {
        m_primary->failNext = true;
        const quint64 id = m_service->notify(note(u"x"_s));
        QCOMPARE(m_primary->shown.size(), 1);
        QCOMPARE(m_fallback->shown.size(), 1);
        QCOMPARE(m_fallback->shown.first().first, id);
        QCOMPARE(m_service->activeCount(), 1);
        // Activation now comes from the fallback.
        QSignalSpy spy(m_service.get(), &NotificationService::activated);
        Q_EMIT m_fallback->activated(id);
        QCOMPARE(spy.count(), 1);
    }

    void usesFallbackWhenPrimaryUnavailable()
    {
        m_primary->available = false;
        QCOMPARE(m_service->backendName(), u"fake"_s);
        m_service->notify(note(u"x"_s));
        QCOMPARE(m_primary->shown.size(), 0);
        QCOMPARE(m_fallback->shown.size(), 1);
    }

    void dropsWhenNothingAvailable()
    {
        m_primary->available = false;
        m_fallback->available = false;
        QCOMPARE(m_service->notify(note(u"x"_s)), quint64{0});
        QCOMPARE(m_service->backendName(), u"none"_s);
    }

    void dndExpires()
    {
        QSignalSpy spy(m_dnd.get(), &DndController::stateChanged);
        m_dnd->enableFor(std::chrono::minutes(0)); // fires on the next event-loop turn
        QVERIFY(m_dnd->isActive());
        QVERIFY(m_dnd->until().has_value());
        QTRY_VERIFY(!m_dnd->isActive());
        QCOMPARE(spy.count(), 2);

        m_dnd->enableIndefinitely();
        QVERIFY(!m_dnd->until().has_value());
        QTest::qWait(20);
        QVERIFY(m_dnd->isActive());
    }

private:
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Settings> m_settings;
    std::unique_ptr<DndController> m_dnd;
    std::unique_ptr<FakeNotifier> m_primary;
    std::unique_ptr<FakeNotifier> m_fallback;
    std::unique_ptr<NotificationService> m_service;
};

QTEST_MAIN(TestNotificationService)
#include "tst_notification_service.moc"
