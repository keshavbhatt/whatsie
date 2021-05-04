#ifndef RATEAPP_H
#define RATEAPP_H

#include <QWidget>
#include <QSettings>
#include <QDateTime>
#include <QUrl>
#include <QDesktopServices>
#include <QTimer>

namespace Ui {
class RateApp;
}

class RateApp : public QWidget
{
    Q_OBJECT

signals:
    void showRateDialog();

public:
    explicit RateApp(QWidget *parent = nullptr , QString app_rating_url = "",
                     int app_launch_count = 5, int app_install_days = 5, int present_delay = 5000);
    ~RateApp();

public slots:
    void delayShowEvent();
protected slots:
    void closeEvent(QCloseEvent *event);
private slots:
    void on_rateNowBtn_clicked();

    void on_alreadyDoneBtn_clicked();

    void on_laterBtn_clicked();

    bool shouldShow();
private:
    Ui::RateApp *ui;
    QString app_rating_url;
    int app_launch_count;
    int app_install_days;
    int present_delay;
    QSettings settings;
    QTimer *showTimer;
};

#endif // RATEAPP_H
