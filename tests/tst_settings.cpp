#include "core/app_lock.h"
#include "core/settings/settings.h"

#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
using whatsie::core::CloseAction;
using whatsie::core::Settings;
using whatsie::core::Theme;

class TestSettings : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init()
    {
        m_dir = std::make_unique<QTemporaryDir>();
        QVERIFY(m_dir->isValid());
        m_settings = std::make_unique<Settings>(m_dir->filePath(u"test.ini"_s));
    }

    void cleanup()
    {
        m_settings.reset();
        m_dir.reset();
    }

    void defaults()
    {
        QCOMPARE(m_settings->zoomFactor(), 1.0);
        QCOMPARE(m_settings->zoomFactorMaximized(), 1.0);
        QCOMPARE(m_settings->theme(), Theme::System);
        QCOMPARE(m_settings->closeAction(), CloseAction::MinimizeToTray);
        QCOMPARE(m_settings->startMinimized(), false);
        QCOMPARE(m_settings->trayLeftClickToggles(), true);
        QCOMPARE(m_settings->smoothScrolling(), false);
        QVERIFY(m_settings->windowGeometry().isEmpty());
        QVERIFY(m_settings->windowState().isEmpty());
        QVERIFY(m_settings->settingsDialogGeometry().isEmpty());
    }

    void zoomFactorRoundTripAndSignal()
    {
        QSignalSpy spy(m_settings.get(), &Settings::zoomFactorChanged);
        m_settings->setZoomFactor(1.5);
        QCOMPARE(m_settings->zoomFactor(), 1.5);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toDouble(), 1.5);

        // Same value again: no signal.
        m_settings->setZoomFactor(1.5);
        QCOMPARE(spy.count(), 1);
    }

    void zoomFactorIsClamped()
    {
        m_settings->setZoomFactor(100.0);
        QCOMPARE(m_settings->zoomFactor(), 5.0);
        m_settings->setZoomFactor(0.0);
        QCOMPARE(m_settings->zoomFactor(), 0.25);
    }

    void zoomFactorMaximizedIsIndependent()
    {
        QSignalSpy spy(m_settings.get(), &Settings::zoomFactorMaximizedChanged);
        m_settings->setZoomFactorMaximized(1.25);
        QCOMPARE(m_settings->zoomFactorMaximized(), 1.25);
        QCOMPARE(m_settings->zoomFactor(), 1.0);
        QCOMPARE(spy.count(), 1);
    }

    void themeRoundTripAndSignal()
    {
        QSignalSpy spy(m_settings.get(), &Settings::themeChanged);
        m_settings->setTheme(Theme::Dark);
        QCOMPARE(m_settings->theme(), Theme::Dark);
        QCOMPARE(spy.count(), 1);
        m_settings->setTheme(Theme::Dark);
        QCOMPARE(spy.count(), 1);
    }

    void boolOptionsEmitOnlyOnChange()
    {
        QSignalSpy start(m_settings.get(), &Settings::startMinimizedChanged);
        QSignalSpy tray(m_settings.get(), &Settings::trayLeftClickTogglesChanged);
        QSignalSpy smooth(m_settings.get(), &Settings::smoothScrollingChanged);

        m_settings->setStartMinimized(false); // default → no signal
        QCOMPARE(start.count(), 0);
        m_settings->setStartMinimized(true);
        QCOMPARE(start.count(), 1);
        QVERIFY(m_settings->startMinimized());

        m_settings->setTrayLeftClickToggles(false);
        QCOMPARE(tray.count(), 1);
        QVERIFY(!m_settings->trayLeftClickToggles());

        m_settings->setSmoothScrolling(true);
        m_settings->setSmoothScrolling(true);
        QCOMPARE(smooth.count(), 1);
    }

    void closeActionRoundTrip()
    {
        QSignalSpy spy(m_settings.get(), &Settings::closeActionChanged);
        m_settings->setCloseAction(CloseAction::Quit);
        QCOMPARE(m_settings->closeAction(), CloseAction::Quit);
        QCOMPARE(spy.count(), 1);
        m_settings->setCloseAction(CloseAction::Quit);
        QCOMPARE(spy.count(), 1);
    }

    void persistsAcrossInstances()
    {
        const QString path = m_settings->fileName();
        m_settings->setZoomFactor(2.0);
        m_settings->setTheme(Theme::Light);
        m_settings->setCloseAction(CloseAction::Quit);
        m_settings->setWindowGeometry(QByteArrayLiteral("geom"));
        m_settings->setSettingsDialogGeometry(QByteArrayLiteral("dlg"));
        m_settings.reset();

        Settings reloaded(path);
        QCOMPARE(reloaded.zoomFactor(), 2.0);
        QCOMPARE(reloaded.theme(), Theme::Light);
        QCOMPARE(reloaded.closeAction(), CloseAction::Quit);
        QCOMPARE(reloaded.windowGeometry(), QByteArrayLiteral("geom"));
        QCOMPARE(reloaded.settingsDialogGeometry(), QByteArrayLiteral("dlg"));
    }

    void garbageValuesFallBackToDefaults()
    {
        const QString path = m_settings->fileName();
        m_settings.reset();
        {
            QSettings raw(path, QSettings::IniFormat);
            raw.setValue(u"appearance/theme"_s, 999);
            raw.setValue(u"window/closeAction"_s, -5);
            raw.setValue(u"view/zoomFactor"_s, u"not a number"_s);
        }
        Settings reloaded(path);
        QCOMPARE(reloaded.theme(), Theme::System);
        QCOMPARE(reloaded.closeAction(), CloseAction::MinimizeToTray);
        QCOMPARE(reloaded.zoomFactor(), 0.25); // toDouble() → 0 → clamped to min
    }

    void lockIdleMinutesClampsHugeStoredValue()
    {
        // A corrupted/hand-edited value must not overflow minutes * 60 * 1000.
        const QString path = m_settings->fileName();
        m_settings.reset();
        {
            QSettings raw(path, QSettings::IniFormat);
            raw.setValue(u"lock/idleMinutes"_s, 999999999);
        }
        Settings reloaded(path);
        QVERIFY(reloaded.lockIdleMinutes() >= 0);
        QVERIFY(reloaded.lockIdleMinutes() <= 1440);
    }

    void resetToDefaultsKeepsLockConfiguration()
    {
        using whatsie::core::makePasscode;
        m_settings->setPasscode(makePasscode(u"1234"_s, 1000));
        m_settings->setLockOnStart(true);
        m_settings->setLockIdleMinutes(15);
        m_settings->setZoomFactor(2.0);

        m_settings->resetToDefaults();

        // Security config survives; everything else reverts.
        QVERIFY(m_settings->hasPasscode());
        QVERIFY(whatsie::core::verifyPasscode(u"1234"_s, m_settings->passcodeRecord()));
        QCOMPARE(m_settings->lockOnStart(), true);
        QCOMPARE(m_settings->lockIdleMinutes(), 15);
        QCOMPARE(m_settings->zoomFactor(), 1.0);
    }

private:
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Settings> m_settings;
};

QTEST_GUILESS_MAIN(TestSettings)
#include "tst_settings.moc"
