/*
  ESP32 app to read Ruuvi tags via BLE, Openweathermap.org forecast via API & display on Nextion display
*/
#include "settings.h"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#include "localtime.h"
#include "nextionInterface.h"
#include "ruuvi.h"
#include "weather.h"

RuuviScan ruuviScan;

owmWeather currentWeather((String)OW_CITY, (float)OW_LAT, (float)OW_LON, (String)OW_API_KEY);
void getWeather();
int weatherIconToNextionPictureLarge(String);
int weatherIconToNextionPictureSmall(String);

Time currentTime;
void uptime();
char uptimeBuffer[12];  // scratch space for storing formatted 'uptime' string

myNextionInterface myNex(NEXTION_SERIAL, NEXTION_BAUD);
void handleNextion(void*);
TaskHandle_t xhandleNextionHandle = NULL;

void heartbeat();
void readRuuvi();

void setup() {
  Serial.begin(115200);
  delay(2000);
  // Start Nextion task
  myNex.begin();  // Initialize Nextion interface
  xTaskCreate(handleNextion, "Nextion Handler", 3000, NULL, 6, &xhandleNextionHandle);

  // Initialize Ruuvi BLE scanner
  ruuviScan.begin();
  delay(1000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setHostname(DEVICE_NAME);
  WiFi.setAutoReconnect(true);
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  // Initialize OTA Update libraries
  ArduinoOTA.setHostname(DEVICE_NAME);
  ArduinoOTA.begin();

  // Start SNTP task
  currentTime.begin();

  Serial.println(DEVICE_NAME + (String) " is Woke");

  // First run - update weather
  getWeather();

}

unsigned long heartbeatMillis = millis();
unsigned long weatherTimerMillis = millis();
unsigned long RTCClockTimerMillis = millis();
bool setRTC = true;  // Track if just rebooted

void loop() {
  ArduinoOTA.handle();
  // Every 30 seconds
  if ((millis() - heartbeatMillis) >= HEARTBEAT_INTERVAL_MILLIS) {
    heartbeat();
    readRuuvi();
    heartbeatMillis = millis();
  }
  // Every OW_SCAN_TIME minutes
  if ((millis() - weatherTimerMillis) >= OW_SCAN_TIME * 60000)
  {
    getWeather();
    weatherTimerMillis = millis();
    // currentWeather.dumpCurrentWeather(&Serial);
  }
  // Set Nextion Real Time Clock on bootup
  tm localTime;
  if (setRTC && WiFi.isConnected() && currentTime.now(&localTime)) {
    myNex.setRTC(localTime);
    setRTC = false;
    Serial.println("RTC set");
  }

  // Every hour, check if 3am, if so set NextionRTC
  // RTC drift is minor - this could be called less often
  // The reason for a 3am check is to handle daylight savings time
  if ((millis() - RTCClockTimerMillis) >= 60 * 60 * 1000) {
    tm localTime;
    if (WiFi.isConnected() && currentTime.now(&localTime)) {
      if (localTime.tm_hour == 3) {
        myNex.setRTC(localTime);
        setRTC = false;
        Serial.println("RTC set");
      }
    }
    RTCClockTimerMillis = millis();
  }
}

//Get weather from OpenWeatherMap
void getWeather() {
  int status = 0;
  if (WiFi.isConnected()) {
    Serial.println("Calling currentWeather()");
    status = currentWeather.updateWeather();
    if (status == 200) {
      // currentWeather.dumpCurrentWeather(&Serial);
      myNex.writeNum((String) "page0.humidity.val", currentWeather.currentHumidity());
      myNex.writeStr((String) "page0.wxDescription.txt", currentWeather.currentWeatherDescription());
      myNex.writeNum((String) "page0.windSpeed.val", currentWeather.currentWindSpeed());
      myNex.writeNum((String) "page0.windDirection.val", currentWeather.currentWindDirection());
      myNex.writeStr((String) "page0.City.txt", currentWeather.cityName());
      myNex.writeCmd((String) "page0.wxIcon.pic=" +
                      (String)weatherIconToNextionPictureLarge(currentWeather.currentWeatherIcon()));

      time_t now = currentWeather.observationTime();
      char str[20];
      strftime(str, sizeof(str), "OW: %a %H:%M", localtime(&now));
      myNex.writeStr("page0.statusTxt.txt", str);
      myNex.writeStr("Setup.WeatherStatus.txt", str);

      // Five daily forecasts
      for (int i = 0; i < 5; i++) {
        myNex.writeStr((String) "page0.dateTime" + (String)(i + 1) + ".txt", currentWeather.forecastDayofWeek(i));
        myNex.writeStr((String) "page0.forecastTxt" + (String)(i + 1) + ".txt",
                        (String)currentWeather.forecastDescription(i));
        myNex.writeNum((String) "page0.forecastMin" + (String)(i + 1) + ".val", currentWeather.forecastTempMin(i));
        myNex.writeNum((String) "page0.forecastMax" + (String)(i + 1) + ".val", currentWeather.forecastTempMax(i));
        myNex.writeCmd((String) "page0.forecastIcon" + (String)(i + 1) +
                        ".pic=" + (String)weatherIconToNextionPictureLarge(currentWeather.forecastIcon(i)));
      }
      // Hourly forecast heading
      for (int i = 0; i < 12; i+=4) {
        myNex.writeStr((String) "Hourly.hour" + (String)(i + 1) + ".txt", currentWeather.hourlyHourofDayText(i));
      }
      // 12 hourly forecasts
      for (int i = 0; i < 12; i++) {
        myNex.writeNum((String) "Hourly.temp" + (String)(i + 1) + ".val", currentWeather.hourlyTemp(i));
        myNex.writeCmd((String) "Hourly.clouds" + (String)(i + 1) +
                        ".pic=" + (String)weatherIconToNextionPictureSmall(currentWeather.hourlyIcon(i)));
        myNex.writeNum((String) "Hourly.pop" + (String)(i + 1) + ".val", currentWeather.hourlyPop(i));

        // Convert rain mm/hr into 0-100 integer for Nextion progress bars
        // Consider 5mm/hr to be full scale
        int rain = (int)(currentWeather.hourlyPcpt(i) * 20);
        // Serial.printf("%i : %2.2f\n",rain,currentWeather.hourlyPcpt(i));
        if (rain > 100)
          rain = 100;
        if(rain > 0 && rain <= 5)
          rain=5;
        myNex.writeNum((String) "Hourly.pcpt" + (String)(i + 1) + ".val", rain);
      }
    } else {
      myNex.writeStr("page0.statusTxt.txt", "OW Call Fail");
      myNex.writeStr("Setup.WeatherStatus.txt", "OW Call Fail");
    }
  } else {
    myNex.writeStr("page0.statusTxt.txt", "Wifi Disconnected");
    myNex.writeStr("Setup.WiFiStatus.txt", "Wifi Disconnected");
  }
}

// Map openweathermap icon strings to Nextion picture ID's
// See: https://openweathermap.org/weather-conditions
// Large Icons (Daily display)
int weatherIconToNextionPictureLarge(String iconTxt) {
  std::map<String, int> iconMap = {
      {"01d", 14},  // Clear day
      {"01n", 11},  // Clear Night
      {"02d", 13},  // Partly Cloudy Day
      {"02n", 15},  // Party Cloudy Night
      {"03d", 5},   // Cloudy Day
      {"03n", 5},   // Cloudy Night
      {"04d", 10},   // Cloudy Daylight
      {"04n", 10},   // Cloudy Night
      {"09d", 4},   // Showers Day
      {"09n", 4},   // Showers Night
      {"10d", 4},   // Rain Day
      {"10n", 4},   // Rain Night
      {"11d", 12},  // Thunderstorm Day
      {"11n", 12},  // Thunderstorm Night
      {"13d", 7},   // Snow Day
      {"13n", 7},   // Snow Night
      {"50d", 0},   // Mist Day
      {"50n", 0},   // Mist Night
  };

  return iconMap[iconTxt];
}

//Small Icons (hourly display)
int weatherIconToNextionPictureSmall(String iconTxt) {
  std::map<String, int> iconMap = {
      {"01d", 1},    // Clear day
      {"01n", 2},    // Clear Night
      {"02d", 8},    // Partly Cloudy Day
      {"02n", 9},    // Party Cloudy Night
      {"03d", 20},   // Cloudy Day
      {"03n", 20},   // Cloudy Night
      {"04d", 21},   // Cloudy Daylight
      {"04n", 21},   // Cloudy Night
      {"09d", 22},   // Showers Day
      {"09n", 22},   // Showers Night
      {"10d", 22},   // Rain Day
      {"10n", 22},   // Rain Night
      {"11d", 23},   // Thunderstorm Day
      {"11n", 23},   // Thunderstorm Night
      {"13d", 24},   // Snow Day
      {"13n", 24},   // Snow Night
      {"50d", 25},   // Mist Day
      {"50n", 25},   // Mist Night
  };

  return iconMap[iconTxt];
}

void readRuuvi() {
  tm timei;
  time_t now = time(&now);
  currentTime.now(&timei);
  Serial.println(asctime(&timei));

  // Start blocking BLE scan
  // Scan results handled by callback
  ruuviScan.startRuuviScan(RUUVI_SCAN_TIME);

  // Update Nextion screen
  // Dim screen objects if more than 10 minutes between Ruuvi reads
  if ((now - indoorTag.lastUpdate()) < 600) {
    myNex.writeNum((String) "page0.indoorTemp.val", indoorTag.getTemperatureInF());
    myNex.writeCmd("page0.indoorTemp.pco=65535");
    vTaskDelay(100 / portTICK_PERIOD_MS);

  } else {
    myNex.writeCmd("page0.indoorTemp.pco=19049");
  }
  if ((now - outdoorTag.lastUpdate()) < 600) {
    myNex.writeNum((String) "page0.outdoorTemp.val", outdoorTag.getTemperatureInF());
    myNex.writeCmd("page0.outdoorTemp.pco=65535");
    vTaskDelay(100 / portTICK_PERIOD_MS);

  } else {
    myNex.writeCmd("page0.outdoorTemp.pco=19049");
  }

  // Update status text on Nextion
  char str[10];
  time_t ot = outdoorTag.lastUpdate();
  strftime(str, sizeof(str), "%a %H:%M", localtime(&ot));
  myNex.writeStr("Setup.OutdoorStatus.txt", (String)str + " T: " + outdoorTag.getTemperatureInF());
  time_t it = indoorTag.lastUpdate();
  strftime(str, sizeof(str), "%a %H:%M", localtime(&it));
  myNex.writeStr("Setup.IndoorStatus.txt", (String)str + " T: " + indoorTag.getTemperatureInF());
}

void heartbeat() {
  uptime();
  // Send heartbeat counter to Nextion
  myNex.writeNum("heartbeat", 1);

  // Send stack/heap infor to Nextion & Serial port
  String status = (String)uptimeBuffer + 
                  " N: " + (String)uxTaskGetStackHighWaterMark(xhandleNextionHandle) +
                  // " W: " + (String)uxTaskGetStackHighWaterMark(currentWeather.xhandlegetWeatherHandle) +
                  " H: " + (String)esp_get_minimum_free_heap_size();
  Serial.println(status);
  myNex.writeStr("Setup.Heartbeat.txt", status);

  // Check WiFi
  myNex.writeStr("Setup.WiFiStatus.txt", (String) "IP: " + WiFi.localIP().toString());
  uint8_t wifiStatus = WiFi.waitForConnectResult();
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    myNex.writeStr("Setup.WiFiStatus.txt", "WiFi Disconnected");
    delay(5000);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

// Read incoming events (messages) from Nextion
void handleNextion(void* parameter) {
  // Events we care about
  const char filter[] = {'\x65', '\x66', '\x67', '\x68', '\x70', '\x71', '\x86', '\x87', '\xAA'};

  std::string _bytes;  // Raw bytes returned from Nextion, incl. \xFF terminaters
  _bytes.reserve(255);  // Size of buffer is arbitrary.

  std::string _hexString;  // _bytes converted to space delimited ASCII chars
                           // I.E. 1A B4 E4 FF FF FF
  char _x[4] = {};

  vTaskDelay(100 / portTICK_PERIOD_MS);
  for (;;) {  // ever
    // Check for incoming event from Nextion
    if (_bytes.length() > 0) _bytes.clear();
    if (_hexString.length() > 0) _hexString.clear();

    int _len = myNex.listen(_bytes, 255);
    if (_len) {
      if (_len > 3) {
        _hexString.reserve(_len * 3);
        for (const auto& item : _bytes) {
          sprintf(_x, "%02X ", item);
          _hexString += _x;
        }
        // Serial.println((String) "handleNextion returned: " + _hexString.c_str());

        // If we see interesting event from Nextion.
        for (size_t i = 0; i < sizeof filter; i++) {
          if (_bytes[0] == filter[i]) {
            // Respond to Nextion Message
            // (do nothing)
          }
        }
      } else {
        myNex.flushReads();
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  Serial.println("Task ended");
  vTaskDelete(NULL);  // Should never reach this.
}  // handleNextion()

// Calculate uptime & populate uptime buffer for future use
void uptime() {
  // Constants for uptime calculations
  static const uint32_t millis_in_day = 1000 * 60 * 60 * 24;
  static const uint32_t millis_in_hour = 1000 * 60 * 60;
  static const uint32_t millis_in_minute = 1000 * 60;

  uint8_t days = millis() / (millis_in_day);
  uint8_t hours = (millis() - (days * millis_in_day)) / millis_in_hour;
  uint8_t minutes = (millis() - (days * millis_in_day) - (hours * millis_in_hour)) / millis_in_minute;
  snprintf(uptimeBuffer, sizeof(uptimeBuffer), "%2dd%2dh%2dm", days, hours, minutes);
}
