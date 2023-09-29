#include <Arduino.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <WiFi.h>

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

myNextionInterface myNex(NEXTION_SERIAL, NEXTION_BAUD);
void handleNextion(void*);
TaskHandle_t xhandleNextionHandle = NULL;

// FastLED
#define NUM_LEDS 1         // Number of RGB LED beads
#define DATA_PIN 27        // The pin for controlling RGB LED
#define LED_TYPE NEOPIXEL  // RGB LED strip type
CRGB leds[NUM_LEDS];       // Instantiate RGB LED

void heartbeat();

void setup() {
  delay(2000);
  Serial.begin(115200);
  Serial.println(DEVICE_NAME + (String) " is Woke");
  delay(2000);

  FastLED.addLeds<SK6812, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  FastLED.setBrightness(127);

  // Start Nextion task
  myNex.begin();  // Initialize Nextion interface
  xTaskCreate(handleNextion, "Nextion Handler", 3000, NULL, 6, &xhandleNextionHandle);

  // Initialize Ruuvi BLE scanner
  ruuviScan.begin();
  delay(1000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setHostname(DEVICE_NAME);
  WiFi.setAutoReconnect(true);
  uint8_t status = WiFi.waitForConnectResult();
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize OTA Update libraries
  ArduinoOTA.setHostname(DEVICE_NAME);
  ArduinoOTA.begin();

  // Start SNTP task
  currentTime.begin();

  // Start Openweather API task
  xTaskCreate(getWeather, "Weather Handler", 6000, NULL, 6, &currentWeather.xhandlegetWeatherHandle);
}

void loop() {
  ArduinoOTA.handle();
  tm timei;
  time_t now = time(&now);
  currentTime.now(&timei);
  Serial.println(asctime(&timei));

  ruuviScan.checkIsRunning();

  // Update Nextion screen
  // Dim screen objects if more than 10 minutes between Ruuvi reads
  if ((now - indoorTag.lastUpdate()) < 600) {
    leds[0] = CRGB::Green;  // LED shows red light
    FastLED.show();
    myNex.writeNum((String) "indoorTemp.val", indoorTag.getTemperatureInF());
    myNex.writeCmd("indoorTemp.pco=65535");
    vTaskDelay(100 / portTICK_PERIOD_MS);
    leds[0] = CRGB(0, 0, 0);  // LED oFF
    FastLED.show();

  } else {
    myNex.writeCmd("indoorTemp.pco=19049");
  }
  if ((now - outdoorTag.lastUpdate()) < 600) {
    leds[0] = CRGB::Blue;  // LED shows red light
    FastLED.show();
    myNex.writeNum((String) "outdoorTemp.val", outdoorTag.getTemperatureInF());
    myNex.writeCmd("outdoorTemp.pco=65535");
    vTaskDelay(100 / portTICK_PERIOD_MS);

    leds[0] = CRGB(0, 0, 0);  // LED oFF
    FastLED.show();
  } else {
    myNex.writeCmd("outdoorTemp.pco=19049");
  }
  heartbeat();
  delay(10000);
}

// Weather thread
// Get weather from OpenWeatherMap every two minutes
void getWeather(void* parameter) {
  int status = 0;
  for (;;) {  // ever

    leds[0] = CRGB::Red;  // LED shows red light
    FastLED.show();

    Serial.println("Calling currentWeather()");
    status = currentWeather.updateCurrentWeather();
    if (status == 200) {
      // currentWeather.dumpCurrentWeather(&Serial);
      myNex.writeNum((String) "humidity.val", currentWeather.getCurrentWeatherHumidity());
      myNex.writeStr((String) "wxDescription.txt", currentWeather.getCurrentWeatherDescription());
      myNex.writeNum((String) "windSpeed.val", currentWeather.getCurrentWeatherWindSpeed());
      myNex.writeNum((String) "windDirection.val", currentWeather.getCurrentWeatherWindDirection());
      myNex.writeStr((String) "City.txt", currentWeather.getCurrentWeatherCityName());
      myNex.writeCmd((String) "wxIcon.pic=" +
                     (String)weatherIconToNextionPicture(currentWeather.getCurrentWeatherIcon()));

      time_t now = currentWeather.getCurrentWeatherObservationTime();
      char str[20];
      strftime(str, sizeof(str), "%a %H:%M", localtime(&now));
      myNex.writeStr("statusTxt.txt", str);

      // Five day forecast
      for (int i = 1; i <= 5; i++) {
        myNex.writeStr((String) "dateTime" + (String)i + ".txt", currentWeather.getForecastObservationDayofWeek(i));
        myNex.writeStr((String) "forecastTxt" + (String)i + ".txt", (String)currentWeather.getForecastDescription(i));
        myNex.writeNum((String) "forecastMin" + (String)i + ".val", currentWeather.getForecastTempMin(i));
        myNex.writeNum((String) "forecastMax" + (String)i + ".val", currentWeather.getForecastTempMax(i));
        myNex.writeCmd((String) "forecastIcon" + (String)i +
                       ".pic=" + (String)weatherIconToNextionPicture(currentWeather.getForecastIcon(i)));
      }
      leds[0] = CRGB(0, 0, 0);  // LED oFF
      FastLED.show();
    } else {
      myNex.writeStr("statusTxt.txt", "OW Call Fail");
    }
    vTaskDelay(120000 / portTICK_PERIOD_MS);
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

void heartbeat() {
  myNex.writeNum("heartbeat", 1);
  Serial.printf("N: %i W: %i H: %i\n", uxTaskGetStackHighWaterMark(xhandleNextionHandle),
                uxTaskGetStackHighWaterMark(currentWeather.xhandlegetWeatherHandle), esp_get_minimum_free_heap_size());
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
