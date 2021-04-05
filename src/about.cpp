#include "about.h"
#include "ui_about.h"
#include <QDesktopServices>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QUrl>

About::About(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    ui->version->setText("Version: "+QApplication::applicationVersion());

    connect(ui->donate,&QPushButton::clicked,[=](){
        QDesktopServices::openUrl(QUrl("https://paypal.me/keshavnrj/10"));
    });

    connect(ui->rate,&QPushButton::clicked,[=](){
        QDesktopServices::openUrl(QUrl("snap://whatsie"));
    });
    connect(ui->more_apps,&QPushButton::clicked,[=](){
        QDesktopServices::openUrl(QUrl("https://snapcraft.io/search?q=keshavnrj"));
    });
    connect(ui->source_code,&QPushButton::clicked,[=](){
        QDesktopServices::openUrl(QUrl("https://github.com/keshavbhatt/whatsie"));
    });

    setWindowTitle(QApplication::applicationName() +" | About");

    ui->centerWidget->hide();

    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    ui->centerWidget->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(1000);
    a->setStartValue(0);
    a->setEndValue(1);
    a->setEasingCurve(QEasingCurve::InCurve);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    ui->centerWidget->show();
}

About::~About()
{
    delete ui;
}
