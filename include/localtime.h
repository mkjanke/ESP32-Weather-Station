#ifndef LOCALTIME_H
#define LOCALTIME_H

/*----------------------------------------------------------------
  Simple interface into ESP32 SNTP time

    Initializes SNTP client on ESP32, provides access to current time as struct and individual elements.
    Uses Arduino libraries.

    begin() must be called once, after WiFi is initialized.

    now(*timeinfo_p) fills in struct tm with current time. Doesn't update stored time.
    now(String &time) updates String with asciitime(). Doe not update stored time.
    storeCurrentTime() saves current time in private tm struct.
    getStoredTime(String &time) retrieves previously stored time.
    hour(), minute() & other time functions return portions of stored time

*/

#include <Arduino.h>
#include <WiFi.h>

class Time {
 private:
  tm storedTime;  // Stores the last time from storeCurrentTime function

 public:
  bool isTimeSet = false;  // Flag to check if time is set

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

  // Save current time in private tm struct.
  bool storeCurrentTime() {
    if (getLocalTime(&storedTime)) {  // If successful, store time
      isTimeSet = true;
      return true;
    } else
      return false;
  }

  // Retrieve previously stored time
  bool getStoredTime(String &time) {
    if (isTimeSet) {
      time = (String)(asctime(&storedTime));
      return true;
    } else
      return false;
  }

  // Access indiviual elements of stored tm struct
  // Check if time has been set with setCurrentTime() function.
  // Return '0' if not set.

  int hour() {
    if (isTimeSet)
      return storedTime.tm_hour;
    else
      return 0;
  }
  int minute() {
    if (isTimeSet)
      return storedTime.tm_min;
    else
      return 0;
  }

  int second() {
    if (isTimeSet)
      return storedTime.tm_sec;
    else
      return 0;
  }

  int day() {
    if (isTimeSet)
      return storedTime.tm_mday;
    else
      return 0;
  }

  int month() {
    if (isTimeSet)
      return storedTime.tm_mon + 1;
    else
      return 0;
  }

  int year() {
    if (isTimeSet)
      return storedTime.tm_year + 1900;
    else
      return 0;
  }

  int dayOfWeek() {
    if (isTimeSet)
      return storedTime.tm_wday;
    else
      return 0;
  }
};

#endif  // LOCALTIME_H
