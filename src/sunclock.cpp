#include <sunclock.hpp>
#include <cmath>
#include <stdexcept>

inline double rad(double degrees) {
  static const double degToRad = 4.0 * atan(1.0) / 180.0;
  return degrees * degToRad;
}

inline double deg(double radians) {
  static const double radToDeg = 180.0 / (4.0 * atan(1.0));
  return radians * radToDeg;
}

Sunclock::Sunclock(double const &latitude_, double const &longitude_,
                   double const &tz_offset_)
    : latitude(latitude_), longitude(longitude_), tz_offset(tz_offset_) {}

double Sunclock::irradiance() { return irradiance(time(0)); }

double Sunclock::irradiance(time_t when) {
  when = when + (time_t)(tz_offset * 60 * 60);
  struct tm *t = gmtime(&when);
  double _time_of_day = time_of_day(when);
  double _julian_day = julian_day(t, _time_of_day, tz_offset);
  double _julian_century = julian_century(_julian_day);
  double _mean_obliq_ecliptic = mean_obliq_ecliptic(_julian_century);
  double _mean_long_sun = mean_long_sun(_julian_century);
  double _mean_anom_sun = mean_anom_sun(_julian_century);
  double _sun_eq_of_centre = sun_eq_of_centre(_mean_anom_sun, _julian_century);
  double _sun_true_long = sun_true_long(_mean_long_sun, _sun_eq_of_centre);
  double _obliq_corr = obliq_corr(_mean_obliq_ecliptic, _julian_century);
  double _sun_app_long = sun_app_long(_sun_true_long, _julian_century);
  double _eccent_earth_orbit = eccent_earth_orbit(_julian_century);
  double _var_y = var_y(_obliq_corr);
  double _eq_of_time =
      eq_of_time(_var_y, _mean_long_sun, _eccent_earth_orbit, _mean_anom_sun);
  double _declination = declination(_obliq_corr, _sun_app_long);
  double _true_solar_time = true_solar_time(_time_of_day, _eq_of_time);
  double _hour_angle = hour_angle(_true_solar_time);
  double _solar_zenith = solar_zenith(_declination, _hour_angle);
  return std::cos(rad(_solar_zenith));
}

time_t Sunclock::sunrise() { return sunrise(time(0)); }

time_t Sunclock::sunrise(time_t date) {
  date = date + (time_t)(tz_offset * 60 * 60);
  struct tm *t = gmtime(&date);
  double _time_of_day = time_of_day(date);
  double _julian_day = julian_day(t, _time_of_day, tz_offset);
  double _julian_century = julian_century(_julian_day);
  double _mean_obliq_ecliptic = mean_obliq_ecliptic(_julian_century);
  double _mean_long_sun = mean_long_sun(_julian_century);
  double _mean_anom_sun = mean_anom_sun(_julian_century);
  double _sun_eq_of_centre = sun_eq_of_centre(_mean_anom_sun, _julian_century);
  double _sun_true_long = sun_true_long(_mean_long_sun, _sun_eq_of_centre);
  double _obliq_corr = obliq_corr(_mean_obliq_ecliptic, _julian_century);
  double _sun_app_long = sun_app_long(_sun_true_long, _julian_century);
  double _eccent_earth_orbit = eccent_earth_orbit(_julian_century);
  double _var_y = var_y(_obliq_corr);
  double _eq_of_time =
      eq_of_time(_var_y, _mean_long_sun, _eccent_earth_orbit, _mean_anom_sun);
  double _declination = declination(_obliq_corr, _sun_app_long);
  double _hour_angle_sunrise = hour_angle_sunrise(_declination);

  double noon_decimal_day =
      (720 - 4 * longitude - _eq_of_time + tz_offset * 60) / 1440;
  double decimal_day = noon_decimal_day - _hour_angle_sunrise * 4 / 1440;
  return time_from_decimal_day(date, decimal_day) -
         (time_t)(tz_offset * 60 * 60);
}

time_t Sunclock::solar_noon() { return solar_noon(time(0)); }

time_t Sunclock::solar_noon(time_t date) {
  date = date + (time_t)(tz_offset * 60 * 60);
  struct tm *t = gmtime(&date);
  double _time_of_day = time_of_day(date);
  double _julian_day = julian_day(t, _time_of_day, tz_offset);
  double _julian_century = julian_century(_julian_day);
  double _mean_obliq_ecliptic = mean_obliq_ecliptic(_julian_century);
  double _mean_long_sun = mean_long_sun(_julian_century);
  double _mean_anom_sun = mean_anom_sun(_julian_century);
  double _obliq_corr = obliq_corr(_mean_obliq_ecliptic, _julian_century);
  double _eccent_earth_orbit = eccent_earth_orbit(_julian_century);
  double _var_y = var_y(_obliq_corr);
  double _eq_of_time =
      eq_of_time(_var_y, _mean_long_sun, _eccent_earth_orbit, _mean_anom_sun);

  double decimal_day =
      (720 - 4 * longitude - _eq_of_time + tz_offset * 60) / 1440;
  return time_from_decimal_day(date, decimal_day) -
         (time_t)(tz_offset * 60 * 60);
}

time_t Sunclock::sunset() { return sunset(time(0)); }

time_t Sunclock::sunset(time_t date) {
  date = date + (time_t)(tz_offset * 60 * 60);
  struct tm *t = gmtime(&date);
  double _time_of_day = time_of_day(date);
  double _julian_day = julian_day(t, _time_of_day, tz_offset);
  double _julian_century = julian_century(_julian_day);
  double _mean_obliq_ecliptic = mean_obliq_ecliptic(_julian_century);
  double _mean_long_sun = mean_long_sun(_julian_century);
  double _mean_anom_sun = mean_anom_sun(_julian_century);
  double _sun_eq_of_centre = sun_eq_of_centre(_mean_anom_sun, _julian_century);
  double _sun_true_long = sun_true_long(_mean_long_sun, _sun_eq_of_centre);
  double _obliq_corr = obliq_corr(_mean_obliq_ecliptic, _julian_century);
  double _sun_app_long = sun_app_long(_sun_true_long, _julian_century);
  double _eccent_earth_orbit = eccent_earth_orbit(_julian_century);
  double _var_y = var_y(_obliq_corr);
  double _eq_of_time =
      eq_of_time(_var_y, _mean_long_sun, _eccent_earth_orbit, _mean_anom_sun);
  double _declination = declination(_obliq_corr, _sun_app_long);
  double _hour_angle_sunrise = hour_angle_sunrise(_declination);

  double noon_decimal_day =
      (720 - 4 * longitude - _eq_of_time + tz_offset * 60) / 1440;
  double decimal_day = noon_decimal_day + _hour_angle_sunrise * 4 / 1440;
  return time_from_decimal_day(date, decimal_day) -
         (time_t)(tz_offset * 60 * 60);
}

double Sunclock::time_of_day(time_t date) {
  struct tm *t = gmtime(&date);
  return (t->tm_hour + t->tm_min / 60.0 + t->tm_sec / 3600.0) / 24.0;
}

time_t Sunclock::time_from_decimal_day(time_t date, double decimal_day) {
  struct std::tm epoch;
  epoch.tm_isdst = 0;
  epoch.tm_sec = epoch.tm_min = epoch.tm_hour = epoch.tm_mon = 0;
  epoch.tm_mday = 1;
  epoch.tm_year = 70;
  time_t local_tz_offset = mktime(&epoch);

  struct tm *dt = gmtime(&date);
  struct tm t = {};
  t.tm_year = dt->tm_year;
  t.tm_mon = dt->tm_mon;
  t.tm_mday = dt->tm_mday;
  double hours = 24.0 * decimal_day;
  t.tm_hour = int(hours);
  double minutes = (hours - t.tm_hour) * 60;
  t.tm_min = int(minutes);
  double seconds = (minutes - t.tm_sec) * 60;
  t.tm_sec = int(seconds) % 60;
  return mktime(&t) - local_tz_offset;
}

int Sunclock::days_since_1900(struct tm *t) {
  int year = t->tm_year;
  if (year < 0 || year > 199) {
    throw std::invalid_argument(
        "days_since_1900 - Date must be between 1900 and 2099");
  }
  int month = t->tm_mon + 1;
  int days = t->tm_mday;

  if (month < 3) {
    month += 12;
    year--;
  }
  int yearDays = (int)(year * 365.25);
  int monthDays = (int)((month + 1) * 30.61);

  return (yearDays + monthDays + days - 63);
}

double Sunclock::julian_day(struct tm *t, double const &time_of_day,
                            double const &tz_offset) {
  return days_since_1900(t) + 2415018.5 + time_of_day - tz_offset / 24;
}

double Sunclock::julian_century(double _julian_day) {
  return (_julian_day - 2451545.0) / 36525.0;
}

double Sunclock::mean_long_sun(double _julian_century) {
  return (
      280.46646 +
      fmod(_julian_century * (36000.76983 + _julian_century * 0.0003032), 360));
}

double Sunclock::mean_anom_sun(double _julian_century) {
  return 357.52911 +
         _julian_century * (35999.05029 - 0.0001537 * _julian_century);
}

double Sunclock::sun_eq_of_centre(double _mean_anom_sun,
                                  double _julian_century) {
  return sin(rad(_mean_anom_sun)) *
             (1.914602 -
              _julian_century * (0.004817 + 0.000014 * _julian_century)) +
         sin(rad(2 * _mean_anom_sun)) *
             (0.019993 - 0.000101 * _julian_century) +
         sin(rad(3 * _mean_anom_sun)) * 0.000289;
}

double Sunclock::sun_true_long(double _mean_long_sun,
                               double _sun_eq_of_centre) {
  return _mean_long_sun + _sun_eq_of_centre;
}

double Sunclock::eccent_earth_orbit(double _julian_century) {
  return 0.016708634 -
         _julian_century * (0.000042037 + 0.0001537 * _julian_century);
}

double Sunclock::var_y(double _obliq_corr) {
  return tan(rad(_obliq_corr / 2)) * tan(rad(_obliq_corr / 2));
}

double Sunclock::mean_obliq_ecliptic(double _julian_century) {
  return (23 + (26 + ((21.448 - _julian_century *
                                    (46.815 + _julian_century *
                                                  (0.00059 - _julian_century *
                                                                 0.001813)))) /
                         60) /
                   60);
}

double Sunclock::obliq_corr(double _mean_obliq_ecliptic,
                            double _julian_century) {
  return _mean_obliq_ecliptic +
         0.00256 * cos(deg(125.04 - 1934.136 * _julian_century));
}

double Sunclock::sun_app_long(double _sun_true_long, double _julian_century) {
  return (_sun_true_long - 0.00569 -
          0.00478 * sin(deg(125.04 - 1934.136 * _julian_century)));
}

double Sunclock::declination(double _obliq_corr, double _sun_app_long) {
  return deg(asin(sin(rad(_obliq_corr)) * sin(rad(_sun_app_long))));
}

double Sunclock::eq_of_time(double _var_y, double _mean_long_sun,
                            double _eccent_earth_orbit, double _mean_anom_sun) {
  return 4 * deg(_var_y * sin(2 * rad(_mean_long_sun)) -
                 2 * _eccent_earth_orbit * sin(rad(_mean_anom_sun)) +
                 4 * _eccent_earth_orbit * _var_y * sin(rad(_mean_anom_sun)) *
                     cos(2 * rad(_mean_long_sun)) -
                 0.5 * _var_y * _var_y * sin(4 * rad(_mean_long_sun)) -
                 1.25 * _eccent_earth_orbit * _eccent_earth_orbit *
                     sin(2 * rad(_mean_anom_sun)));
}

double Sunclock::true_solar_time(double _time_of_day, double _eq_of_time) {
  return fmod(
      (_time_of_day * 1440 + _eq_of_time + 4 * longitude - 60 * tz_offset),
      1440);
}

double Sunclock::hour_angle(double _true_solar_time) {
  return (_true_solar_time / 4 < 0 ? _true_solar_time / 4 + 180
                                   : _true_solar_time / 4 - 180);
}

double Sunclock::hour_angle_sunrise(double _declination) {
  return deg(
      acos(cos(rad(90.833)) / (cos(rad(latitude)) * cos(rad(_declination))) -
           tan(rad(latitude)) * tan(rad(_declination))));
}

double Sunclock::solar_zenith(double _declination, double _hour_angle) {
  return deg(acos(sin(rad(latitude)) * sin(rad(_declination)) +
                  cos(rad(latitude)) * cos(rad(_declination)) *
                      cos(rad(_hour_angle))));
}

double Sunclock::solar_elevation(double _solar_zenith) {
  return 90 - _solar_zenith;
}
