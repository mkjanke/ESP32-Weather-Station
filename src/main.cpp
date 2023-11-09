/*
  ESP32 app to read Ruuvi tags vi BLE, Openweathermap.org forecast via API & display on Nextion display
*/
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#include "led.h"
#include "localtime.h"
#include "nextionInterface.h"
#include "ruuvi.h"
#include "settings.h"
#include "weather.h"

RuuviScan ruuviScan;

owmWeather currentWeather((String)OW_CITY, (float)OW_LAT, (float)OW_LON, (String)OW_API_KEY);
void getWeather(void* parameter);
int weatherIconToNextionPicture(String);

Time currentTime;
void uptime();
char uptimeBuffer[12];  // scratch space for storing formatted 'uptime' string

myNextionInterface myNex(NEXTION_SERIAL, NEXTION_BAUD);
void handleNextion(void*);
TaskHandle_t xhandleNextionHandle = NULL;

Led led;

void heartbeat();
void readRuuvi();

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println(DEVICE_NAME + (String) " is Woke");

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

  // Start Openweather API task
  xTaskCreate(getWeather, "Weather Handler", 6000, NULL, 6, &currentWeather.xhandlegetWeatherHandle);
}

unsigned long lastMillis = millis();
bool firstRun = true;  // Track if just rebooted

void loop() {
  ArduinoOTA.handle();
  // Every 30 seconds
  if (millis() > lastMillis + 30000) {
    heartbeat();
    readRuuvi();
    lastMillis = millis();

    // Set Nextion Real Time Clock on bootup
    String localTime;
    if (firstRun && currentTime.now(localTime)) {
      myNex.writeNum((String) "rtc0", currentTime.year());
      myNex.writeNum((String) "rtc1", currentTime.month());
      myNex.writeNum((String) "rtc2", currentTime.day());
      myNex.writeNum((String) "rtc3", currentTime.hour());
      myNex.writeNum((String) "rtc4", currentTime.minute());
      firstRun = false;
    }
  }
}

// Weather thread
// Get weather from OpenWeatherMap every 'n' minutes
void getWeather(void* parameter) {
  int status = 0;
  for (;;) {  // ever
    if (WiFi.isConnected()) {
      led.showRed();
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
                       (String)weatherIconToNextionPicture(currentWeather.currentWeatherIcon()));

        time_t now = currentWeather.observationTime();
        char str[20];
        strftime(str, sizeof(str), "OW: %a %H:%M", localtime(&now));
        myNex.writeStr("page0.statusTxt.txt", str);
        myNex.writeStr("Setup.WeatherStatus.txt", str);

        // Five day forecast
        for (int i = 0; i < 5; i++) {
          myNex.writeStr((String) "page0.dateTime" + (String)(i + 1) + ".txt", currentWeather.forecastDayofWeek(i));
          myNex.writeStr((String) "page0.forecastTxt" + (String)(i + 1) + ".txt",
                         (String)currentWeather.forecastDescription(i));
          myNex.writeNum((String) "page0.forecastMin" + (String)(i + 1) + ".val", currentWeather.forecastTempMin(i));
          myNex.writeNum((String) "page0.forecastMax" + (String)(i + 1) + ".val", currentWeather.forecastTempMax(i));
          myNex.writeCmd((String) "page0.forecastIcon" + (String)(i + 1) +
                         ".pic=" + (String)weatherIconToNextionPicture(currentWeather.forecastIcon(i)));
        }
        led.clear();
      } else {
        myNex.writeStr("page0.statusTxt.txt", "OW Call Fail");
        myNex.writeStr("Setup.WeatherStatus.txt", "OW Call Fail");
      }
    } else {
      myNex.writeStr("page0.statusTxt.txt", "Wifi Disconnected");
      myNex.writeStr("Setup.WiFiStatus.txt", "Wifi Disconnected");
    }
    vTaskDelay((OW_SCAN_TIME * 60000) / portTICK_PERIOD_MS);
  }
}

// Map openweathermap icon strings to Nextion picture ID's
// See: https://openweathermap.org/weather-conditions
int weatherIconToNextionPicture(String iconTxt) {
  std::map<String, int> iconMap = {
      {"01d", 14},  // Clear day
      {"01n", 11},  // Clear Night
      {"02d", 13},  // Partly Cloudy Day
      {"02n", 15},  // Party Cloudy Night
      {"03d", 5},   // Cloudy Day
      {"03n", 5},   // Cloudy Night
      {"04d", 5},   // Cloudy Daylight
      {"04n", 5},   // Cloudy Night
      {"09d", 1},   // Showers Day
      {"09d", 3},   // Showers Night
      {"10d", 4},   // Rain Day
      {"10n", 8},   // Rain Night
      {"11d", 12},  // Thunderstorm Day
      {"11n", 10},  // Thunderstorm Night
      {"13d", 7},   // Snow Day
      {"13n", 9},   // Snow Night
      {"50d", 0},   // Mist Day
      {"50n", 0},   // Mist Night
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
    led.showGreen();
    myNex.writeNum((String) "page0.indoorTemp.val", indoorTag.getTemperatureInF());
    myNex.writeCmd("page0.indoorTemp.pco=65535");
    vTaskDelay(100 / portTICK_PERIOD_MS);
    led.clear();

  } else {
    myNex.writeCmd("page0.indoorTemp.pco=19049");
  }
  if ((now - outdoorTag.lastUpdate()) < 600) {
    led.showBlue();
    myNex.writeNum((String) "page0.outdoorTemp.val", outdoorTag.getTemperatureInF());
    myNex.writeCmd("page0.outdoorTemp.pco=65535");
    vTaskDelay(100 / portTICK_PERIOD_MS);
    led.clear();

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
  String status = (String)uptimeBuffer + " N: " + (String)uxTaskGetStackHighWaterMark(xhandleNextionHandle) +
                  " W: " + (String)uxTaskGetStackHighWaterMark(currentWeather.xhandlegetWeatherHandle) +
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
  _bytes.reserve(48);  // Size of buffer is arbitrary.

  std::string _hexString;  // _bytes converted to space delimited ASCII chars
                           // I.E. 1A B4 E4 FF FF FF
  char _x[3] = {};

  vTaskDelay(100 / portTICK_PERIOD_MS);
  for (;;) {  // ever
    // Check for incoming event from Nextion
    if (_bytes.length() > 0) _bytes.clear();
    if (_hexString.length() > 0) _hexString.clear();

    int _len = myNex.listen(_bytes, 48);
    if (_len) {
      if (_len > 3) {
        _hexString.reserve(_len * 3);
        for (const auto& item : _bytes) {
          sprintf(_x, "%02X ", item);
          _hexString += _x;
        }
        Serial.println((String) "handleNextion returned: " + _hexString.c_str());

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
