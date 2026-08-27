#include "core/settings/settings.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
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
        QCOMPARE(m_settings->theme(), Theme::System);
        QVERIFY(m_settings->windowGeometry().isEmpty());
        QVERIFY(m_settings->windowState().isEmpty());
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

    void themeRoundTripAndSignal()
    {
        QSignalSpy spy(m_settings.get(), &Settings::themeChanged);
        m_settings->setTheme(Theme::Dark);
        QCOMPARE(m_settings->theme(), Theme::Dark);
        QCOMPARE(spy.count(), 1);
        m_settings->setTheme(Theme::Dark);
        QCOMPARE(spy.count(), 1);
    }

    void persistsAcrossInstances()
    {
        const QString path = m_settings->fileName();
        m_settings->setZoomFactor(2.0);
        m_settings->setTheme(Theme::Light);
        m_settings->setWindowGeometry(QByteArrayLiteral("geom"));
        m_settings.reset();

        Settings reloaded(path);
        QCOMPARE(reloaded.zoomFactor(), 2.0);
        QCOMPARE(reloaded.theme(), Theme::Light);
        QCOMPARE(reloaded.windowGeometry(), QByteArrayLiteral("geom"));
    }

    void garbageThemeFallsBackToSystem()
    {
        const QString path = m_settings->fileName();
        m_settings.reset();
        {
            QSettings raw(path, QSettings::IniFormat);
            raw.setValue(u"appearance/theme"_s, 999);
        }
        Settings reloaded(path);
        QCOMPARE(reloaded.theme(), Theme::System);
    }

private:
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Settings> m_settings;
};

QTEST_GUILESS_MAIN(TestSettings)
#include "tst_settings.moc"
