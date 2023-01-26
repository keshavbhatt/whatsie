#ifndef DAYLIGHT_SUNCLOCK_HPP
#define DAYLIGHT_SUNCLOCK_HPP

#include <ctime>

class Sunclock {
public:
  Sunclock(double const &latitude_, double const &longitude_, double const &tz_offset_ = 0);

  /**
   * \overload double Sunclock::irradiance(time_t when)
   */
  double irradiance();

  /**
   * Specifically, we calculate Direct Normal Irradiance
   * That is we do not consider atmospheric components like cloud cover,
   * moisture content etc.
   *
   * @param when
   * @return a value representing irradiance (power per unit area)
   */
  double irradiance(time_t when);

  /**
   * \overload time_t Sunclock::sunrise(time_t date)
   */
  time_t sunrise();

  /**
   * Returns sunrise time for given date
   *
   * @param date only date is considered
   * @return sunrise time
   */
  time_t sunrise(time_t date);

  /**
   * \overload time_t Sunclock::solar_noon(time_t date)
   */
  time_t solar_noon();

  /**
   * Returns solar_noon time for given date
   *
   * @param date only date is considered
   * @return solar_noon time
   */
  time_t solar_noon(time_t date);

  /**
   * \overload time_t Sunclock::sunset(time_t date)
   */
  time_t sunset();

  /**
   * Returns sunset time for given date
   *
   * @param date only date is considered
   * @return sunset time
   */
  time_t sunset(time_t date);

private:
  // in decimal degrees, east is positive
  double const latitude;

  // in decimal degrees, north is positive
  double const longitude;

  // in hours, east is positive, i.e. IST (+05:30) is 5.5
  double const tz_offset;

  /**
   * @param date
   * @return percentage past midnight, i.e. noon  is 0.5
   */
  double time_of_day(time_t date);

  static int days_since_1900(struct tm *t);

  /**
   * @param date
   * @param decimal_day decimal between 0.0 and 1.0, e.g. noon = 0.5
   * @return time_t with date and time set accordingly
   */
  static time_t time_from_decimal_day(time_t date, double decimal_day);

  /**
   * Get day count since Monday, January 1, 4713 BC
   * https://en.wikipedia.org/wiki/Julian_day
   *
   * @param t
   * @param time_of_day percentage past midnight, i.e. noon is 0.5
   * @param tz_offset   in hours, east is positive, i.e. IST (+05:30) is 5.5
   * @return julian day
   */
  static double julian_day(struct tm *t, double const &time_of_day, double const &tz_offset);

  /**
   * Extract information in a suitable format from a time_t object.
   */
  void prepare_time(time_t const &when, struct tm *ptm);

  double julian_century(double _julian_day);
  double mean_long_sun(double _julian_century);
  double mean_anom_sun(double _julian_century);
  double sun_eq_of_centre(double _mean_anom_sun, double _julian_century);
  double eccent_earth_orbit(double _julian_century);
  double var_y(double _obliq_corr);
  double mean_obliq_ecliptic(double _julian_century);
  double sun_true_long(double _mean_long_sun, double _sun_eq_of_centre);
  double obliq_corr(double _mean_obliq_ecliptic, double _julian_century);
  double sun_app_long(double _sun_true_long, double _julian_century);
  double declination(double _obliq_corr, double _sun_app_long);
  double eq_of_time(double _var_y, double _mean_long_sun, double _eccent_earth_orbit,
                    double _mean_anom_sun);
  double true_solar_time(double _time_of_day, double _eq_of_time);
  double hour_angle(double _true_solar_time);
  double hour_angle_sunrise(double _declination);
  double solar_zenith(double _declination, double _hour_angle);
  double solar_elevation(double _solar_zenith);
};

#endif

