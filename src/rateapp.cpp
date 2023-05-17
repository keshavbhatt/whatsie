#include "rateapp.h"
#include "ui_rateapp.h"
#include <QDebug>
#include <QMetaEnum>

RateApp::RateApp(QWidget *parent, QString app_rating_url, int app_launch_count,
                 int app_install_days, int present_delay)
    : QWidget(parent), ui(new Ui::RateApp) {
  ui->setupUi(this);

  this->app_rating_url =
      app_rating_url; // Url to open when rating button clicked
  this->app_launch_count =
      app_launch_count; // How many time the app must be launched by user to
                        // show this dialog
  this->app_install_days =
      app_install_days; // How many days the app must be installed by user to
                        // show this dialog
  this->present_delay =
      present_delay; // Delay after which this dialog should be shown to use if
                     // all conditions matched

  showTimer = new QTimer(this);
  showTimer->setInterval(this->present_delay);
  connect(showTimer, &QTimer::timeout, this, [=]() {
    qDebug() << "Rate timer timeout";
    emit showRateDialog();
    if (this->isVisible())
      showTimer->stop();
  });

  // increase the app_launched_count by one
  int app_launched = SettingsManager::instance()
                         .settings()
                         .value("app_launched_count", 0)
                         .toInt();
  SettingsManager::instance().settings().setValue("app_launched_count",
                                                  app_launched + 1);

  // check if app install time is set in settings
  if (SettingsManager::instance()
          .settings()
          .value("app_install_time")
          .isNull()) {
    SettingsManager::instance().settings().setValue(
        "app_install_time", QDateTime::currentSecsSinceEpoch());

  } else if (SettingsManager::instance()
                 .settings()
                 .value("app_install_time")
                 .isValid()) {
    // qDebug() << "RATEAPP should show:" << shouldShow();
    if (shouldShow()) {
      showTimer->start();
    } else {
      // if shouldshow is false, delete this obj to free resources
      showTimer->stop();
      this->deleteLater();
    }
  }

  // if already reated delete this obj to free resources
  if (SettingsManager::instance()
          .settings()
          .value("rated_already", false)
          .toBool()) {
    this->deleteLater();
  }
}

void RateApp::delayShowEvent() {
  showTimer->start();

  qDebug() << "Timer running" << showTimer->isActive() << showTimer->interval()
           << showTimer->isSingleShot();
  qDebug() << "App was minimized and was not visible, "
              "hence delaying the showevent of RateApp dialog by "
           << this->showTimer->interval() / 1000 << "seconds";
}

/**
 * @brief RateApp::shouldShow
 * @return true, if the dialog should be shown to user
 */
bool RateApp::shouldShow() {
  bool shouldShow = false;
  int app_launched_count = SettingsManager::instance()
                               .settings()
                               .value("app_launched_count", 0)
                               .toInt();
  qint64 currentDateTime = QDateTime::currentSecsSinceEpoch();
  qint64 installed_date_time = SettingsManager::instance()
                                   .settings()
                                   .value("app_install_time")
                                   .toLongLong();
  bool ratedAlready = SettingsManager::instance()
                          .settings()
                          .value("rated_already", false)
                          .toBool();

  if (ratedAlready) // return false if already reated;
    return false;

  shouldShow =
      (((currentDateTime - installed_date_time > app_install_days * 86400) ||
        app_launched_count >= this->app_launch_count) &&
       ratedAlready == false);

  return shouldShow;
}

RateApp::~RateApp() {
  showTimer->disconnect();
  showTimer->deleteLater();
  delete ui;
}

void RateApp::on_rateNowBtn_clicked() {
  QDesktopServices::openUrl(QUrl(app_rating_url));
  this->reset();
  this->close();
}

void RateApp::on_alreadyDoneBtn_clicked() {
  SettingsManager::instance().settings().setValue("rated_already", true);
  this->close();
}

void RateApp::on_laterBtn_clicked() {
  this->reset();
  this->close();
}

void RateApp::reset() {
  SettingsManager::instance().settings().setValue("rated_already", false);
  SettingsManager::instance().settings().setValue("app_launched_count", 0);
  SettingsManager::instance().settings().setValue(
      "app_install_time", QDateTime::currentSecsSinceEpoch());
}

void RateApp::on_rateOnGithub_clicked() {
  QDesktopServices::openUrl(QUrl("https://github.com/keshavbhatt/whatsie"));
  this->reset();
  this->close();
}

void RateApp::on_donate_clicked() {
  QDesktopServices::openUrl(QUrl("https://paypal.me/keshavnrj/5.00"));
  this->reset();
  this->close();
}

void RateApp::on_donate_2_clicked() {
  QDesktopServices::openUrl(QUrl("https://opencollective.com/whatsie"));
  this->reset();
  this->close();
}
