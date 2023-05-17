#ifndef RATEAPP_H
#define RATEAPP_H

#include <QDateTime>
#include <QDesktopServices>
#include <QTimer>
#include <QUrl>
#include <QWidget>

#include "settingsmanager.h"

namespace Ui {
class RateApp;
}

class RateApp : public QWidget {
  Q_OBJECT

signals:
  void showRateDialog();

public:
  explicit RateApp(QWidget *parent = nullptr, QString app_rating_url = "",
                   int app_launch_count = 5, int app_install_days = 5,
                   int present_delay = 5000);
  ~RateApp();

public slots:
  void delayShowEvent();

private slots:
  void on_rateNowBtn_clicked();

  void on_alreadyDoneBtn_clicked();

  void on_laterBtn_clicked();

  bool shouldShow();
  void reset();
  void on_rateOnGithub_clicked();

  void on_donate_clicked();

  void on_donate_2_clicked();

private:
  Ui::RateApp *ui;
  QString app_rating_url;
  int app_launch_count;
  int app_install_days;
  int present_delay;
  QTimer *showTimer;
};

#endif // RATEAPP_H
