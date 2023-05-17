#include "automatictheme.h"
#include "ui_automatictheme.h"

#include "sunclock.hpp"

AutomaticTheme::AutomaticTheme(QWidget *parent)
    : QWidget(parent), ui(new Ui::AutomaticTheme) {
  ui->setupUi(this);
  ui->refresh->setEnabled(false);

  m_sunrise.setSecsSinceEpoch(
      SettingsManager::instance().settings().value("sunrise").toLongLong());
  m_sunset.setSecsSinceEpoch(
      SettingsManager::instance().settings().value("sunset").toLongLong());

  ui->sunrise->setTime(m_sunrise.time());
  ui->sunset->setTime(m_sunset.time());

  QTimeZone zone = QTimeZone::systemTimeZone();
  QDateTime dt = QDateTime::currentDateTime();

  if (zone.isValid()) {
    m_hourOffset = (double)zone.standardTimeOffset(dt) / (double)3600;
  } else {
    SettingsManager::instance().settings().setValue("automaticTheme", false);
    QMessageBox::critical(
        this, "Error",
        "Unable to get system TimeZone information.\n\nAutomatic theme "
        "switcher will not work.");
    return;
  }

  m_gPosInfoSrc = QGeoPositionInfoSource::createDefaultSource(this);

  if (m_gPosInfoSrc) // sudo apt install geoclue-2.0
  {
    ui->refresh->setEnabled(true);
    connect(m_gPosInfoSrc, &QGeoPositionInfoSource::positionUpdated, this,
            [=](const QGeoPositionInfo &update) {
              QGeoCoordinate cor = update.coordinate();
              if (cor.isValid()) {
                this->m_longitude = cor.longitude();
                this->m_latitube = cor.latitude();
                ui->refresh->setEnabled(true);
                m_gPosInfoSrc->stopUpdates();
              } else {
                ui->refresh->setEnabled(false);
              }
            });
    connect(m_gPosInfoSrc, &QGeoPositionInfoSource::updateTimeout, this, [=]() {
      if (!SettingsManager::instance().settings().value("sunrise").isValid() ||
          !SettingsManager::instance().settings().value("sunset").isValid()) {
        if (ui->refresh->isEnabled())
          ui->refresh->click();
      }
    });
    m_gPosInfoSrc->startUpdates();
  } else {
    ui->refresh->setEnabled(false);
    SettingsManager::instance().settings().setValue("automaticTheme", false);
    QMessageBox::critical(
        this, "Error",
        "Unable to initialize QGeoPositionInfoSource.\n\nAutomatic theme "
        "switcher will not work."
        "\n\nPlease fill the sunset and sunrise time manually.");
  }
}

AutomaticTheme::~AutomaticTheme() {
  m_gPosInfoSrc->disconnect();
  m_gPosInfoSrc->deleteLater();
  delete ui;
}

void AutomaticTheme::on_refresh_clicked() {
  QGeoCoordinate geoCor = QGeoCoordinate(this->m_latitube, this->m_longitude);
  if (geoCor.isValid()) {
    Sunclock sun(this->m_latitube, this->m_longitude, this->m_hourOffset);
    m_sunrise.setSecsSinceEpoch(
        sun.sunrise(QDateTime::currentDateTimeUtc().toTime_t()));
    m_sunset.setSecsSinceEpoch(
        sun.sunset(QDateTime::currentDateTimeUtc().toTime_t()));

    ui->sunrise->setTime(m_sunrise.time());
    ui->sunset->setTime(m_sunset.time());
  } else {
    SettingsManager::instance().settings().setValue("automaticTheme", false);
    QMessageBox::critical(this, "Error",
                          "Invalid Geo-Coordinates.\n\nPlease try again.");
  }
}

void AutomaticTheme::on_save_clicked() {
  if (m_sunrise.toSecsSinceEpoch() == m_sunset.toSecsSinceEpoch()) {
    SettingsManager::instance().settings().setValue("automaticTheme", false);
    QMessageBox::critical(
        this, "Error",
        "Invalid SettingsManager::instance().settings().\n\nSunrise and Sunset "
        "time cannot "
        "have similar values.\n\nPlease try again.");
  } else {
    SettingsManager::instance().settings().setValue(
        "sunrise", m_sunrise.toSecsSinceEpoch());
    SettingsManager::instance().settings().setValue(
        "sunset", m_sunset.toSecsSinceEpoch());
    SettingsManager::instance().settings().setValue("automaticTheme", true);
    this->close();
  }
}

void AutomaticTheme::on_cancel_clicked() {
  SettingsManager::instance().settings().setValue("automaticTheme", false);
  this->close();
}

void AutomaticTheme::on_sunrise_timeChanged(const QTime &time) {
  m_sunrise.setTime(QTime(time.hour(), time.minute(), 0));
}

void AutomaticTheme::on_sunset_timeChanged(const QTime &time) {
  m_sunset.setTime(QTime(time.hour(), time.minute(), 0));
}
