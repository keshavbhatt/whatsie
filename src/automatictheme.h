#ifndef AUTOMATICTHEME_H
#define AUTOMATICTHEME_H

#include <QDebug>
#include <QGeoPositionInfoSource>
#include <QMessageBox>
#include <QTimeZone>
#include <QWidget>

#include "settingsmanager.h"

namespace Ui {
class AutomaticTheme;
}

class AutomaticTheme : public QWidget {
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

  QGeoPositionInfoSource *m_gPosInfoSrc;
  QDateTime m_sunrise, m_sunset;

  double m_hourOffset = 0.0;
  double m_longitude = 0.0;
  double m_latitube = 0.0;
};

#endif // AUTOMATICTHEME_H
