#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#include "localtime.h"
#include "nextionInterface.h"
#include "ruuvi.h"
#include "settings.h"
#include "weather.h"

RuuviScan ruuviScan;

owmWeather currentWeather;

Time currentTime;

myNextionInterface myNex(NEXTION_SERIAL, NEXTION_BAUD);
void handleNextion(void*);
TaskHandle_t xhandleNextionHandle = NULL;

void getWeather(void* parameter);
void heartbeat();

void setup() {
  delay(2000);
  Serial.begin(115200);
  Serial.println(DEVICE_NAME + (String) " is Woke");
  delay(2000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setHostname(DEVICE_NAME);
  uint8_t status = WiFi.waitForConnectResult();

  // Initialize OTA Update libraries
  ArduinoOTA.setHostname(DEVICE_NAME);
  ArduinoOTA.begin();

  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  currentTime.begin();

  myNex.begin();  // Initialize Nextion interface
  xTaskCreate(handleNextion, "Nextion Handler", 3000, NULL, 6, &xhandleNextionHandle);

  ruuviScan.begin();
  xTaskCreate(getWeather, "Weather Handler", 5000, NULL, 6, &currentWeather.xhandlegetWeatherHandle);
}

void loop() {
  ArduinoOTA.handle();
  tm timei;
  time_t now = time(&now);
  currentTime.now(&timei);
  Serial.println(asctime(&timei));

  ruuviScan.checkIsRunning();
  if((now - indoorTag.lastUpdate()) < 600 ){
    myNex.writeNum((String) "indoorTemp.val", indoorTag.getTemperatureInF());
    myNex.writeCmd("indoorTemp.pco=65535");
  } else{
    myNex.writeCmd("indoorTemp.pco=19049");
  }
  if((now - outdoorTag.lastUpdate()) < 600 ){
    myNex.writeNum((String) "outdoorTemp.val", outdoorTag.getTemperatureInF());
    myNex.writeCmd("outdoorTemp.pco=65535");
  } else{
    myNex.writeCmd("outdoorTemp.pco=19049");
  }
  myNex.writeNum((String) "humidity.val", currentWeather.getHumidity());
  myNex.writeNum((String) "atmPressure.val", currentWeather.getPressure());
  myNex.writeStr((String) "wxDescription.txt", currentWeather.getDescription());
  myNex.writeStr((String) "weatherIcon.txt", currentWeather.getIcon());
  myNex.writeNum((String) "windSpeed.val", currentWeather.getWindSpeed());
  myNex.writeNum((String) "windDirection.val", currentWeather.getWindDirection());
  myNex.writeStr((String) "City.txt", currentWeather.getCityName());

  heartbeat();
  delay(10000);
}

// Weather thread
// Get weather from OpenWeatherMap every two minutes
void getWeather(void* parameter) {
  for (;;) {  // ever
    Serial.println("Calling currentWeather()");
    currentWeather.getCurrentWeather();
    currentWeather.dump(&Serial);
    vTaskDelay(120000 / portTICK_PERIOD_MS);
  }
}

void heartbeat() {
  myNex.writeNum("heartbeat", 1);
  Serial.printf("N: %i W: %i H: %i\n", 
                uxTaskGetStackHighWaterMark(xhandleNextionHandle),
                uxTaskGetStackHighWaterMark(currentWeather.xhandlegetWeatherHandle), 
                esp_get_minimum_free_heap_size());
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
