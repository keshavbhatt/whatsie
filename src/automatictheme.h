#ifndef AUTOMATICTHEME_H
#define AUTOMATICTHEME_H

#include <QWidget>
#include <QGeoPositionInfoSource>
#include <QDebug>
#include <QTimeZone>
#include <QSettings>

namespace Ui {
class AutomaticTheme;
}

class AutomaticTheme : public QWidget
{
    Q_OBJECT

public:
    explicit AutomaticTheme(QWidget *parent = nullptr);
    ~AutomaticTheme();


private slots:
    void on_refresh_clicked();

    void on_save_clicked();

    void on_cancel_clicked();

    void on_sunrise_timeChanged(const QTime &time);

    void on_sunset_timeChanged(const QTime &time);

private:
    Ui::AutomaticTheme *ui;

    QGeoPositionInfoSource *gPosInfoSrc;
    QDateTime sunrise, sunset;

    double hour_offset = 0.0;
    double lon = 0.0;
    double lat = 0.0;

    QSettings settings;
};

#endif // AUTOMATICTHEME_H
