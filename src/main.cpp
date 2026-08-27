#include "app/application.h"
#include "ui/main_window.h"

#include <QCoreApplication>

int main(int argc, char* argv[])
{
    // Required by Qt WebEngine before the QApplication exists.
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    whatsie::app::Application app(argc, argv);

    whatsie::ui::MainWindow window(app.settings());
    window.show();

    return whatsie::app::Application::exec();
}
