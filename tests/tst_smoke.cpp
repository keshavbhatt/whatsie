// Smoke test: the whole widget stack constructs, shows, and tears down
// offscreen without crashing. Runs under QT_QPA_PLATFORM=offscreen.

#include "core/settings/settings.h"
#include "core/theme/theme_service.h"
#include "ui/main_window.h"

#include <QApplication>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;

class TestSmoke : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void mainWindowConstructsAndShows()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        whatsie::core::Settings settings(dir.filePath(u"smoke.ini"_s));
        whatsie::core::ThemeService theme(settings);

        whatsie::ui::MainWindow window(settings, theme);
        window.start(false);
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTest::qWait(200);

        // Exercise a few window-level paths that need no network.
        settings.setTheme(whatsie::core::Theme::Dark);
        settings.setZoomFactor(1.5);
        window.toggleVisibility();
        window.showAndRaise();
        QTest::qWait(50);
        window.close();
    }
};

int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);
    QApplication::setApplicationName(u"whatsie-smoke"_s);
    QApplication::setOrganizationName(u"ktechpit"_s);
    QApplication::setQuitOnLastWindowClosed(false);
    TestSmoke test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_smoke.moc"
