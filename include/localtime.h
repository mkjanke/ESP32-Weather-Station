#ifndef LOCALTIME_H
#define LOCALTIME_H

/*----------------------------------------------------------------
  Simple interface into ESP32 SNTP time

    Initializes SNTP client on ESP32, provides access to current time as struct, and as individual elements.

    begin() must be called once, after WiFi is initialized.

*/

#include <WiFi.h>

#include "esp_sntp.h"
#include "time.h"

#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.nist.gov"
#define TZ_STRING "CST6CDT,M3.2.0,M11.1.0"  // Central time, America/Chicago

class Time {
 private:
 public:
  // Call once, after WiFi is initialized.
  void begin() { configTzTime(TZ_STRING, NTP_SERVER_1, NTP_SERVER_2); }

  // Pass struct tm. Fill in struct with current time.
  bool now(tm *timeinfo_p) { return getLocalTime(timeinfo_p); }

  // Pass String. Update String with asciitime()
  bool now(String &time) {
    tm timeinfo;
    bool retVal;
    retVal = getLocalTime(&timeinfo);
    time = (String)(asctime(&timeinfo));
    return retVal;
  }

  // Access indiviual elements of tm struct
  int hour() {
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_hour;
  }
  int minute() {
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_min;
  }

  int second() {
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_sec;
  }

  int day() {
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_mday;
  }

  int month() {  //
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_mon + 1;
  }

  int year() {
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_year + 1900;
  }

  int dayOfWeek() {
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_wday;
  }
};

#endif  // LOCALTIME_H
