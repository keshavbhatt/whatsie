#include "about.h"
#include "ui_about.h"
#include <QDesktopServices>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QUrl>
#include <utils.h>

About::About(QWidget *parent) : QWidget(parent), ui(new Ui::About) {
  ui->setupUi(this);

  // init
  appName = QApplication::applicationName();
  appDescription = "WhatsApp Web clinet for Linux Desktop";
  isOpenSource = true;
  appAuthorName = "Keshav Bhatt";
  appAuthorEmail = "keshavnrj@gmail.com";
  appAuthorLink = "http://ktechpit.com";
  donateLink = "https://paypal.me/keshavnrj/5";
  moreAppsLink = "https://snapcraft.io/search?q=keshavnrj";

  appSourceCodeLink = "https://github.com/keshavbhatt/whatsie";
  appRateLink = "snap://whatsie";

  ui->appNameDesc->setText(
      QString("<p style=' margin-top:12px; margin-bottom:12px; margin-left:0px;"
              " margin-right:0px; -qt-block-indent:0; text-indent:0px;'>"
              "<span style=' font-size:18pt;'>%1</span></p>"
              "<p style=' margin-top:12px; margin-bottom:12px; margin-left:0px;"
              " margin-right:0px; -qt-block-indent:0; text-indent:0px;'>"
              "%2</p>")
          .arg(appName, appDescription));

  ui->desc2->setText(
      QString("<p><span style=' font-weight:600;'>Designed &amp; Developed "
              "by:</span>"
              " %1 </p><p><span style=' font-weight:600;'>"
              "Developer Email address: </span>%2</p>"
              "<p><span style=' font-weight:600;'>Developer Website:</span>"
              " %3</p>")
          .arg(appAuthorName, appAuthorEmail, appAuthorLink));

  ui->version->setText("Version: " + QApplication::applicationVersion());

  ui->debugInfoText->setHtml(utils::appDebugInfo());

  ui->debugInfoText->hide();

  ui->debugInfoButton->setText(QObject::tr("Show Debug Info"));

  if (isOpenSource == false) {
    ui->source_code->hide();
  }

  connect(ui->donate, &QPushButton::clicked,
          [=]() { QDesktopServices::openUrl(QUrl(donateLink)); });

  connect(ui->rate, &QPushButton::clicked,
          [=]() { QDesktopServices::openUrl(QUrl(appRateLink)); });
  connect(ui->more_apps, &QPushButton::clicked,
          [=]() { QDesktopServices::openUrl(QUrl(moreAppsLink)); });
  connect(ui->source_code, &QPushButton::clicked,
          [=]() { QDesktopServices::openUrl(QUrl(appSourceCodeLink)); });

  setWindowTitle(QApplication::applicationName() + " | About");

  ui->centerWidget->hide();

  QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
  ui->centerWidget->setGraphicsEffect(eff);
  QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
  a->setDuration(1000);
  a->setStartValue(0);
  a->setEndValue(1);
  a->setEasingCurve(QEasingCurve::InCurve);
  a->start(QPropertyAnimation::DeleteWhenStopped);
  ui->centerWidget->show();
}

About::~About() { delete ui; }

void About::on_debugInfoButton_clicked() {
  if (ui->debugInfoText->isVisible()) {
    ui->debugInfoText->hide();
    ui->debugInfoButton->setText(QObject::tr("Show Debug Info"));

    this->resize(this->width(), this->minimumHeight());
  } else {
    ui->debugInfoText->show();
    ui->debugInfoButton->setText(QObject::tr("Hide Debug Info"));
    this->adjustSize();
  }
}

void About::on_donate_2_clicked() {
  QDesktopServices::openUrl(QUrl("https://opencollective.com/whatsie"));
}
