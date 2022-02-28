#include "automatictheme.h"
#include "ui_automatictheme.h"

#include "SunClock.hpp"
#include <QMessageBox>

AutomaticTheme::AutomaticTheme(QWidget *parent)
    : QWidget(parent), ui(new Ui::AutomaticTheme) {
  ui->setupUi(this);
  ui->refresh->setEnabled(false);

  sunrise.setSecsSinceEpoch(settings.value("sunrise").toLongLong());
  sunset.setSecsSinceEpoch(settings.value("sunset").toLongLong());

  ui->sunrise->setTime(sunrise.time());
  ui->sunset->setTime(sunset.time());

  QTimeZone zone = QTimeZone::systemTimeZone();
  QDateTime dt = QDateTime::currentDateTime();

  if (zone.isValid()) {
    hour_offset = (double)zone.standardTimeOffset(dt) / (double)3600;
  } else {
    settings.setValue("automaticTheme", false);
    QMessageBox::critical(
        this, "Error",
        "Unable to get system TimeZone information.\n\nAutomatic theme "
        "switcher will not work.");
    return;
  }

  gPosInfoSrc = QGeoPositionInfoSource::createDefaultSource(this);

  if (gPosInfoSrc) // sudo apt install geoclue-2.0
  {
    ui->refresh->setEnabled(true);
    connect(gPosInfoSrc, &QGeoPositionInfoSource::positionUpdated,
            [=](const QGeoPositionInfo &update) {
              QGeoCoordinate cor = update.coordinate();
              if (cor.isValid()) {
                this->lon = cor.longitude();
                this->lat = cor.latitude();
                ui->refresh->setEnabled(true);
                gPosInfoSrc->stopUpdates();
              } else {
                ui->refresh->setEnabled(false);
              }
            });
    connect(gPosInfoSrc, &QGeoPositionInfoSource::updateTimeout, [=]() {
      if (!settings.value("sunrise").isValid() ||
          !settings.value("sunset").isValid()) {
        if (ui->refresh->isEnabled())
          ui->refresh->click();
      }
    });
    gPosInfoSrc->startUpdates();
  } else {
    ui->refresh->setEnabled(false);
    settings.setValue("automaticTheme", false);
    QMessageBox::critical(
        this, "Error",
        "Unable to initialize QGeoPositionInfoSource.\n\nAutomatic theme "
        "switcher will not work."
        "\n\nPlease fill the sunset and sunrise time manually.");
  }
}

AutomaticTheme::~AutomaticTheme() {
  gPosInfoSrc->disconnect();
  gPosInfoSrc->deleteLater();
  delete ui;
}

void AutomaticTheme::on_refresh_clicked() {
  QGeoCoordinate geoCor = QGeoCoordinate(this->lat, this->lon);
  if (geoCor.isValid()) {
    Sunclock sun(this->lat, this->lon, this->hour_offset);
    sunrise.setSecsSinceEpoch(
        sun.sunrise(QDateTime::currentDateTime().toTime_t()));
    sunset.setSecsSinceEpoch(
        sun.sunset(QDateTime::currentDateTime().toTime_t()));

    ui->sunrise->setTime(sunrise.time());
    ui->sunset->setTime(sunset.time());
  } else {
    settings.setValue("automaticTheme", false);
    QMessageBox::critical(this, "Error",
                          "Invalid Geo-Coordinates.\n\nPlease try again.");
  }
}

void AutomaticTheme::on_save_clicked() {
  if (sunrise.toSecsSinceEpoch() == sunset.toSecsSinceEpoch()) {
    settings.setValue("automaticTheme", false);
    QMessageBox::critical(this, "Error",
                          "Invalid settings.\n\nSunrise and Sunset time cannot "
                          "have similar values.\n\nPlease try again.");
  } else {
    settings.setValue("sunrise", sunrise.toSecsSinceEpoch());
    settings.setValue("sunset", sunset.toSecsSinceEpoch());
    settings.setValue("automaticTheme", true);
    this->close();
  }
}

void AutomaticTheme::on_cancel_clicked() {
  settings.setValue("automaticTheme", false);
  this->close();
}

void AutomaticTheme::on_sunrise_timeChanged(const QTime &time) {
  sunrise.setTime(QTime(time.hour(), time.minute(), 0));
}

void AutomaticTheme::on_sunset_timeChanged(const QTime &time) {
  sunset.setTime(QTime(time.hour(), time.minute(), 0));
}
