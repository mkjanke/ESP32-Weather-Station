#ifndef WEATHER_H
#define WEATHER_H
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "settings.h"
#include "time.h"

/*----------------------------------------------------------------
owmWeather object: Wrapper for OpenWeather API call (onecall API, version 3.0)

Returns current weather + 8-day forecast, filtered for minimum required data

Example:

 curl
"https://api.openweathermap.org/data/3.0/onecall?exclude=minutely,hourly,&units=imperial&lon=-93&lat=45&appid=xxxxxxxxxxxxx"

  Sample return (filtered using ArduinoJSON filter):

    {
      "lat": 45,
      "lon": -93,
      "current": {
        "dt": 1695674616,
        "temp": 62.26,
        "feels_like": 62.6,
        "pressure": 1017,
        "humidity": 94,
        "clouds": 100,
        "wind_speed": 6.91,
        "wind_deg": 100,
        "weather": [
          {
            "id": 804,
            "main": "Clouds",
            "description": "overcast clouds",
            "icon": "04d"
          }
        ]
      },
      "daily": [
        {
          "dt": 1695664800,
          "temp": {
            "min": 58.21,
            "max": 63.21
          },
          "weather": [
            {
              "id": 501,
              "main": "Rain",
              "description": "moderate rain",
              "icon": "10d"
            }
          ]
        },
        {
          "dt": 1695751200,
          "temp": {
            "min": 58.64,
            "max": 61.92
          },
          "weather": [
            {
              "id": 500,
              "main": "Rain",
              "description": "light rain",
              "icon": "10d"
            }
          ]
        },
        {
          "dt": 1695837600,
          "temp": {
            "min": 55.35,
            "max": 66.25
          },
          "weather": [
            {
              "id": 500,
              "main": "Rain",
              "description": "light rain",
              "icon": "10d"
            }
          ]
        },
        {
          "dt": 1695924000,
          "temp": {
            "min": 59.76,
            "max": 74.35
          },
          "weather": [
            {
              "id": 802,
              "main": "Clouds",
              "description": "scattered clouds",
              "icon": "03d"
            }
          ]
        },
        {
          "dt": 1696010400,
          "temp": {
            "min": 59.5,
            "max": 86.49
          },
          "weather": [
            {
              "id": 501,
              "main": "Rain",
              "description": "moderate rain",
              "icon": "10d"
            }
          ]
        },
        {
          "dt": 1696096800,
          "temp": {
            "min": 70.03,
            "max": 91.56
          },
          "weather": [
            {
              "id": 803,
              "main": "Clouds",
              "description": "broken clouds",
              "icon": "04d"
            }
          ]
        },
        {
          "dt": 1696183200,
          "temp": {
            "min": 70.2,
            "max": 91.54
          },
          "weather": [
            {
              "id": 802,
              "main": "Clouds",
              "description": "scattered clouds",
              "icon": "03d"
            }
          ]
        },
        {
          "dt": 1696269600,
          "temp": {
            "min": 70.65,
            "max": 91.33
          },
          "weather": [
            {
              "id": 500,
              "main": "Rain",
              "description": "light rain",
              "icon": "10d"
            }
          ]
        }
      ]
    }
*/

struct CurrentWeather {
  float lon;               // "lon": 8.54,
  float lat;               // "lat": 47.37
  uint16_t weatherId;      // "id": 521,
  String main;             // "main": "Rain",
  String description;      // "description": "shower rain",
  String icon;             // "icon": "09d"
  float temp;              // "temp": 290.56,
  uint16_t pressure;       // "pressure": 1013,
  uint8_t humidity;        // "humidity": 87,
  uint8_t feelsLike;       // "feelsLike: 63.78"
  float windSpeed;         // "wind": {"speed": 1.5},
  float windDeg;           // "wind": {deg: 226.505},
  uint8_t clouds;          // "clouds": {"all": 90},
  time_t observationTime;  // "dt": 1527015000,
  String cityName;         // "name": "Zurich",
};

struct ForecastWeather {
  time_t observationTime;  // "dt": 1527015000,
  float tempMin;           // "temp": 290.56,
  float tempMax;           // "temp": 290.56,
  uint16_t weatherId;      // "id": 521,
  String main;             // "main": "Rain",
  String description;      // "description": "shower rain",
  String icon;             // "icon": "09d"
};

class owmWeather {
 private:
  String _cityName;
  float _latitude;
  float _longitude;
  HTTPClient http;
  CurrentWeather weatherNow;       // Current Weather via API 3.0
  ForecastWeather forecast[8];     // Forecast - Eight day forecast availabe in API 3.0
  StaticJsonDocument<768> filter;  // ArduinoJSON Filter Document

  String currentWeatherHost;

 public:
  TaskHandle_t xhandlegetWeatherHandle = NULL;

  owmWeather(String cityName, float lat, float lon, String owAPIKey) {
    _cityName = cityName;
    _latitude = lat;
    _longitude = lon;
    currentWeatherHost = "http://api.openweathermap.org/data/3.0/onecall?appid=" + owAPIKey + "&lat=" + _latitude +
                         "&lon=" + _longitude + "&units=imperial";

    // Populate ArduinoJSON filter document
    // Reduce size of ArduinoJSON 'doc' document
    filter["lon"] = true;
    filter["lat"] = true;
    filter["current"]["weather"][0]["id"] = true;
    filter["current"]["weather"][0]["main"] = true;
    filter["current"]["weather"][0]["icon"] = true;
    filter["current"]["weather"][0]["description"] = true;
    filter["current"]["temp"] = true;
    filter["current"]["feels_like"] = true;
    filter["current"]["pressure"] = true;
    filter["current"]["humidity"] = true;
    filter["current"]["wind_speed"] = true;
    filter["current"]["wind_deg"] = true;
    filter["current"]["clouds"] = true;
    filter["current"]["dt"] = true;

    filter["daily"][0]["dt"] = true;
    filter["daily"][0]["temp"]["min"] = true;
    filter["daily"][0]["temp"]["max"] = true;
    filter["daily"][0]["weather"][0]["id"] = true;
    filter["daily"][0]["weather"][0]["main"] = true;
    filter["daily"][0]["weather"][0]["description"] = true;
    filter["daily"][0]["weather"][0]["icon"] = true;

    // serializeJsonPretty(filter, Serial);
  }

  // Call Openweather API
  // Populate CurrentWeather and ForecastWeather structs
  int updateCurrentWeather() {
    http.useHTTP10(true);
    http.begin(currentWeatherHost);
    // Send HTTP GET request
    int httpResponseCode = http.GET();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      StaticJsonDocument<2048> doc;
      deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
      // Serial.println("JSON DOC:");
      // serializeJsonPretty(doc, Serial);

      // Extract JSON to struct elements
      weatherNow.lon = doc["lon"].as<float>();
      weatherNow.lat = doc["lat"].as<float>();
      weatherNow.weatherId = doc["current"]["weather"][0]["id"].as<unsigned int>();
      weatherNow.main = doc["current"]["weather"][0]["main"].as<String>();
      weatherNow.description = doc["current"]["weather"][0]["description"].as<String>();
      weatherNow.icon = doc["current"]["weather"][0]["icon"].as<String>();
      weatherNow.temp = doc["current"]["temp"].as<float>();
      weatherNow.feelsLike = doc["current"]["feels_like"].as<float>();
      weatherNow.pressure = doc["current"]["pressure"].as<unsigned int>();
      weatherNow.humidity = doc["current"]["humidity"].as<unsigned int>();
      weatherNow.windSpeed = doc["current"]["wind_speed"].as<float>();
      weatherNow.windDeg = doc["current"]["wind_deg"].as<float>();
      weatherNow.clouds = doc["current"]["clouds"].as<unsigned int>();
      weatherNow.observationTime = doc["current"]["dt"].as<time_t>();

      // Now the 8-day forecast forecast
      for (int i = 0; i < 8; i++) {
        forecast[i].observationTime = doc["daily"][i]["dt"].as<time_t>();
        forecast[i].tempMin = doc["daily"][i]["temp"]["min"].as<float>();
        forecast[i].tempMax = doc["daily"][i]["temp"]["max"].as<float>();
        forecast[i].weatherId = doc["daily"][i]["weather"][0]["id"].as<unsigned int>();
        forecast[i].main = doc["daily"][i]["weather"][0]["main"].as<String>();
        forecast[i].description = doc["daily"][i]["weather"][0]["description"].as<String>();
        forecast[i].icon = doc["daily"][i]["weather"][0]["icon"].as<String>();
      }

    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
    return httpResponseCode;
  }

  String getCurrentWeatherDescription() { return weatherNow.description; }
  int getCurrentWeatherOutdoorTemp() { return (int)weatherNow.temp; }
  int getCurrentWeatherHumidity() { return (int)weatherNow.humidity; }
  int getCurrentWeatherPressure() { return (int)weatherNow.pressure; }
  String getCurrentWeatherIcon() { return weatherNow.icon; }
  int getCurrentWeatherWindSpeed() { return (int)weatherNow.windSpeed; }
  int getCurrentWeatherWindDirection() { return (int)weatherNow.windDeg; }
  String getCurrentWeatherCityName() { return _cityName; }
  time_t getCurrentWeatherObservationTime() { return weatherNow.observationTime; }

  time_t getForecastObservationTime(int i) { return forecast[i].observationTime; }
  String getForecastObservationDayofWeek(int i) {
    char buffer[20];
    struct tm *timeinfo;
    timeinfo = localtime(&forecast[i].observationTime);
    strftime(buffer, 20, "%a %d", timeinfo);
    return (String)buffer;
  }
  String getForecastDescription(int i) { return forecast[i].description; }
  int getForecastTempMin(int i) { return (int)(forecast[i].tempMin + (forecast[i].tempMin >= 0 ? .5 : -.5)); }
  int getForecastTempMax(int i) { return (int)(forecast[i].tempMax + (forecast[i].tempMax >= 0 ? .5 : -.5)); }
  int getForecastWeatherId(int i) { return (int)forecast[i].weatherId; }
  String getForecastIcon(int i) { return forecast[i].icon; }
  String getForecastMain(int i) { return forecast[i].main; }

  void dumpCurrentWeather(Stream *_stream) {
    char scratch[26];
    _stream->println();
    _stream->println("lon : " + (String)weatherNow.lon);
    _stream->println("lat : " + (String)weatherNow.lat);
    _stream->println("id : " + (String)weatherNow.weatherId);
    _stream->println("main : " + weatherNow.main);
    _stream->println("description : " + weatherNow.description);
    _stream->println("icon : " + weatherNow.icon);
    _stream->println("temp : " + (String)weatherNow.temp);
    _stream->println("feelsLike : " + (String)weatherNow.feelsLike);
    _stream->println("pressure : " + (String)weatherNow.pressure);
    _stream->println("humidity : " + (String)weatherNow.humidity);
    _stream->println("wind speed : " + (String)weatherNow.windSpeed);
    _stream->println("wind dir : " + (String)weatherNow.windDeg);
    _stream->println("clouds : " + (String)weatherNow.clouds);
    _stream->print("timestamp : " + (String)ctime_r(&weatherNow.observationTime, scratch));
    _stream->println("city : " + (String)OW_CITY);
    _stream->println();

    for (int i = 0; i < 8; i++) {
      _stream->print("timestamp: " + (String)ctime_r(&forecast[i].observationTime, scratch));
      _stream->println("temp min: " + (String)forecast[i].tempMin);
      _stream->println("temp max: " + (String)forecast[i].tempMax);
      _stream->println("id: " + (String)forecast[i].weatherId);
      _stream->println("main: " + (String)forecast[i].main);
      _stream->println("decription: " + (String)forecast[i].description);
      _stream->println("icon: " + (String)forecast[i].icon);
      _stream->println();
    }
  }
};

#endif /* WEATHER_H */