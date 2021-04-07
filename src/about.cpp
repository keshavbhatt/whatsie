#include "about.h"
#include "ui_about.h"
#include <QDesktopServices>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QUrl>
#include <utils.h>

About::About(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    ui->debugInfoText->setHtml(utils::appDebugInfo());
    ui->debugInfoText->hide();
    ui->debugInfoButton->setText(QObject::tr("Show Debug Info"));


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

void About::on_debugInfoButton_clicked()
{
    if(ui->debugInfoText->isVisible()){
        ui->debugInfoText->hide();
        ui->debugInfoButton->setText(QObject::tr("Show Debug Info"));
        //update geometry after above loop is finished
        QTimer::singleShot(300,this,[=]{
            this->resize(this->width(),this->minimumHeight());
        });
    }else{
        ui->debugInfoText->show();
        ui->debugInfoButton->setText(QObject::tr("Hide Debug Info"));
        this->adjustSize();
    }

}
