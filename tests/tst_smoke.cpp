// Smoke test: the whole widget stack constructs, shows, and tears down
// offscreen without crashing. Runs under QT_QPA_PLATFORM=offscreen.

#include "app/application.h"
#include "core/settings/settings.h"
#include "ui/main_window.h"

#include <QTest>

class TestSmoke : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void mainWindowConstructsAndShows()
    {
        whatsie::core::Settings settings(QDir::tempPath() + QStringLiteral("/whatsie-smoke.ini"));
        whatsie::ui::MainWindow window(settings);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTest::qWait(200);
        window.close();
    }
};

int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    whatsie::app::Application app(argc, argv);
    TestSmoke test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_smoke.moc"
