#include "core/settings/settings.h"
#include "core/theme/theme_service.h"

#include <QGuiApplication>
#include <QSignalSpy>
#include <QStyleHints>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
using whatsie::core::Settings;
using whatsie::core::Theme;
using whatsie::core::ThemeService;

class TestThemeService : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void followsExplicitSetting()
    {
        QTemporaryDir dir;
        Settings settings(dir.filePath(u"s.ini"_s));
        settings.setTheme(Theme::Dark);

        ThemeService service(settings);
        QCOMPARE(service.effectiveScheme(), Qt::ColorScheme::Dark);
        QVERIFY(service.isDark());

        QSignalSpy spy(&service, &ThemeService::effectiveSchemeChanged);
        settings.setTheme(Theme::Light);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(service.effectiveScheme(), Qt::ColorScheme::Light);

        settings.setTheme(Theme::Light); // no change → no signal
        QCOMPARE(spy.count(), 1);
    }

    void systemResolvesToPlatformOrLight()
    {
        QTemporaryDir dir;
        Settings settings(dir.filePath(u"s.ini"_s));
        settings.setTheme(Theme::System);
        ThemeService service(settings);

        const Qt::ColorScheme platform = QGuiApplication::styleHints()->colorScheme();
        const Qt::ColorScheme expected =
            platform == Qt::ColorScheme::Unknown ? Qt::ColorScheme::Light : platform;
        QCOMPARE(service.effectiveScheme(), expected);
    }

    void schemeProbeOverridesQtForSystem()
    {
        // The injected desktop probe (the portal on Linux) is authoritative for
        // System mode, since QStyleHints is unreliable there (#367, #374).
        QTemporaryDir dir;
        Settings settings(dir.filePath(u"s.ini"_s));
        settings.setTheme(Theme::System);
        ThemeService service(settings);

        QSignalSpy spy(&service, &ThemeService::effectiveSchemeChanged);
        service.setSchemeProbe([] { return std::optional<Qt::ColorScheme>(Qt::ColorScheme::Dark); });
        QCOMPARE(service.effectiveScheme(), Qt::ColorScheme::Dark);
        QVERIFY(service.isDark());

        // A probe that cannot decide falls back to Qt's value (never crashes).
        service.setSchemeProbe([] { return std::optional<Qt::ColorScheme>(); });
        const Qt::ColorScheme platform = QGuiApplication::styleHints()->colorScheme();
        const Qt::ColorScheme expected =
            platform == Qt::ColorScheme::Unknown ? Qt::ColorScheme::Light : platform;
        QCOMPARE(service.effectiveScheme(), expected);

        // An explicit setting ignores the probe entirely.
        settings.setTheme(Theme::Light);
        service.setSchemeProbe([] { return std::optional<Qt::ColorScheme>(Qt::ColorScheme::Dark); });
        QCOMPARE(service.effectiveScheme(), Qt::ColorScheme::Light);
        QVERIFY(spy.count() >= 1);
    }

    void palettesDiffer()
    {
        QVERIFY(ThemeService::lightPalette().color(QPalette::Window) !=
                ThemeService::darkPalette().color(QPalette::Window));
        // Both accents are WhatsApp's brand green (a brighter shade in dark, a
        // darker one in light for contrast) — green-dominant, not identical.
        for (const QColor accent : {ThemeService::darkPalette().color(QPalette::Highlight),
                                    ThemeService::lightPalette().color(QPalette::Highlight)}) {
            QVERIFY(accent.green() > accent.red() && accent.green() > accent.blue());
        }
    }
};

QTEST_MAIN(TestThemeService)
#include "tst_theme_service.moc"
