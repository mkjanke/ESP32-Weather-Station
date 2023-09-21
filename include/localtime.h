#ifndef LOCALTIME_H
#define LOCALTIME_H

#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"

/*----------------------------------------------------------------
  Simple interface into SNTP time

    Initializes SNTP client on ESP32, provides access to current time as struct, and as inidividual elements.
  
    begin() must be called once, after WiFi is initialized.

*/

#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.nist.gov"
#define TZ_STRING "CST6CDT,M3.2.0,M11.1.0"  // Central time, America/Chicago

class Time {
  private:
    // const char* ntpServer1 = "pool.ntp.org";
    // const char* ntpServer2 = "time.nist.gov";
    // struct tm timeinfo;
        /*
        struct tm
        {
        int    tm_sec;   //   Seconds [0,60]. 
        int    tm_min;   //   Minutes [0,59]. 
        int    tm_hour;  //   Hour [0,23]. 
        int    tm_mday;  //   Day of month [1,31]. 
        int    tm_mon;   //   Month of year [0,11]. 
        int    tm_year;  //   Years since 1900. 
        int    tm_wday;  //   Day of week [0,6] (Sunday =0). 
        int    tm_yday;  //   Day of year [0,365]. 
        int    tm_isdst; //   Daylight Savings flag. 
        }
        */  

  public:

  // Call once, after WiFi is initialized.
  void begin(){
    configTzTime(TZ_STRING, NTP_SERVER_1, NTP_SERVER_2);
  }

  // Pass struct tm. Fill in struct with current time.
  void now(tm *timeinfo_p) { 
    getLocalTime(timeinfo_p);
    return;
  }

  // Pass String. Update String with asciitime()
  void now(String &time) {
    tm timeinfo;
    getLocalTime(&timeinfo);
    time = (String)(asctime(&timeinfo));
  }

  // Access indiviual elements of tm struct
  int hour(){
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_hour;
  }
  int minute(){
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_min;
  }

  int second(){
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_sec;
  }

  int day(){
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_mday;
  }

  int month(){ //
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_mon;
  }

  int year(){
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_year + 1900;
  }

  int dayOfWeek(){
    tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo.tm_wday;
  }

};

#endif // LOCALTIME_H
