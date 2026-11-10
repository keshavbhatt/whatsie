// SPDX-FileCopyrightText: 2026 Keshav Bhatt (Ktechpit)
// SPDX-License-Identifier: MIT
//
// ktechpit_apps_demo [host-id] [grab.png]: an About-like window with the strip, for trying the
// module. With a file name it saves a picture of the window and of the full list, then quits.
#include "AppCatalog.h"
#include "AppStrip.h"
#include "MoreAppsDialog.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

using namespace Qt::StringLiterals;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    const QString host = app.arguments().value(1, u"whatsie"_s);

    ktechpit::CatalogConfig config;
    config.hostAppId = host;
    config.source = host;
    ktechpit::AppCatalog::init(config);

    QWidget window;
    window.setWindowTitle(u"About %1 (KtechpitApps demo)"_s.arg(host));
    auto* layout = new QVBoxLayout(&window);
    layout->addWidget(new QLabel(u"<h2>%1</h2><p>Version 1.0</p>"_s.arg(host)));
    layout->addWidget(new ktechpit::AppStrip(u"about"_s, 3));
    auto* all = new QPushButton(u"More apps…"_s);
    QObject::connect(all, &QPushButton::clicked, &window, [&window] {
        ktechpit::MoreAppsDialog(u"menu"_s, &window).exec();
    });
    layout->addWidget(all);
    window.resize(460, 420);
    window.show();

    const QString grab = app.arguments().value(2);
    if (!grab.isEmpty()) {
        QTimer::singleShot(2500, &window, [&window, grab] {
            window.grab().save(grab);
            auto* dialog = new ktechpit::MoreAppsDialog(u"menu"_s, &window);
            dialog->show();
            QTimer::singleShot(1500, dialog, [dialog, grab] {
                dialog->grab().save(QString(grab).replace(u".png"_s, u"-all.png"_s));
                QApplication::quit();
            });
        });
    }
    return app.exec();
}
