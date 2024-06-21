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

struct DailyForecast {
  time_t observationTime;  // "dt": 1527015000,
  float tempMin;           // "temp": 290.56,
  float tempMax;           // "temp": 290.56,
  uint16_t weatherId;      // "id": 521,
  String main;             // "main": "Rain",
  String description;      // "description": "shower rain",
  String icon;             // "icon": "09d"
};

struct HourlyForecast {
  time_t observationTime;  // "dt": 1527015000,
  float temp   ;           // "temp": 290.56,
  uint8_t clouds;          // "clouds": 90,
  float pop;               // "pop": .2,
  uint16_t weatherId;      // "id": 521,
  String icon;             // "icon": "09d"
  float pcpt;              // "rain "1h" .10
};

class owmWeather {
 private:
  String _cityName;
  float _latitude;
  float _longitude;
  HTTPClient http;
  CurrentWeather weatherNow;            // Current Weather via API 3.0
  DailyForecast dailyForecast[8];       // Forecast - Eight day daily forecast availabe in API 3.0
  HourlyForecast hourlyForecast[24];    // Forecast - 48 hour hourly forecast availabe in API 3.0
  JsonDocument filter;                  // ArduinoJSON Filter Document

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

    filter["hourly"][0]["dt"] = true;
    filter["hourly"][0]["temp"] = true;
    filter["hourly"][0]["clouds"] = true;
    filter["hourly"][0]["pop"] = true;
    filter["hourly"][0]["weather"][0]["icon"] = true;
    filter["hourly"][0]["rain"]["1h"] = true;

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
  // Populate CurrentWeather and DailyForecast structs
  int updateWeather() {
    http.useHTTP10(true);
    Serial.println(currentWeatherHost);
    http.begin(currentWeatherHost);
    // Send HTTP GET request
    int httpResponseCode = http.GET();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      JsonDocument doc;
      DeserializationError err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
      if (err) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(err.c_str());
      } else {
        if (!doc.overflowed()) {
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
          // Populate 8-day forecast
          for (int i = 0; i < 8; i++) {
            dailyForecast[i].observationTime = doc["daily"][i]["dt"].as<time_t>();
            dailyForecast[i].tempMin = doc["daily"][i]["temp"]["min"].as<float>();
            dailyForecast[i].tempMax = doc["daily"][i]["temp"]["max"].as<float>();
            dailyForecast[i].weatherId = doc["daily"][i]["weather"][0]["id"].as<unsigned int>();
            dailyForecast[i].main = doc["daily"][i]["weather"][0]["main"].as<String>();
            dailyForecast[i].description = doc["daily"][i]["weather"][0]["description"].as<String>();
            dailyForecast[i].icon = doc["daily"][i]["weather"][0]["icon"].as<String>();
          }

          // Populate hourly forecast
          for (int i = 0; i < 24; i++) {
            hourlyForecast[i].observationTime = doc["hourly"][i]["dt"].as<time_t>();
            hourlyForecast[i].temp = doc["hourly"][i]["temp"].as<float>();
            hourlyForecast[i].clouds = doc["hourly"][i]["clouds"].as<unsigned int>();
            hourlyForecast[i].weatherId = doc["hourly"][i]["weather"][0]["id"].as<unsigned int>();
            hourlyForecast[i].icon = doc["hourly"][i]["weather"][0]["icon"].as<String>();
            hourlyForecast[i].pop = doc["hourly"][i]["pop"].as<float>();
            if (doc["hourly"][i]["pop"].isNull() )
                hourlyForecast[i].pcpt = 0.0;
              else
                hourlyForecast[i].pcpt = doc["hourly"][i]["rain"]["1h"].as<float>();
          }
        } else {
          Serial.println("ERROR: not enough memory to store the entire document");
        }
      }
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
    return httpResponseCode;
  }

  String currentWeatherDescription() { return weatherNow.description; }
  int currentOutdoorTemp() { return (int)weatherNow.temp; }
  int currentHumidity() { return (int)weatherNow.humidity; }
  int currentAtmPressure() { return (int)weatherNow.pressure; }
  String currentWeatherIcon() { return weatherNow.icon; }
  int currentWindSpeed() { return (int)weatherNow.windSpeed; }
  int currentWindDirection() { return (int)weatherNow.windDeg; }
  String cityName() { return _cityName; }
  time_t observationTime() { return weatherNow.observationTime; }

  // Methods to get daily forecast data
  time_t forecastObservationTime(int i) { return dailyForecast[i].observationTime; }
  String forecastDayofWeek(int i) {
    char buffer[20];
    struct tm *timeinfo;
    timeinfo = localtime(&dailyForecast[i].observationTime);
    strftime(buffer, 20, "%a %d", timeinfo);
    return (String)buffer;
  }
  String forecastDescription(int i) { return dailyForecast[i].description; }
  int forecastTempMin(int i) { return (int)(dailyForecast[i].tempMin + (dailyForecast[i].tempMin >= 0 ? .5 : -.5)); }
  int forecastTempMax(int i) { return (int)(dailyForecast[i].tempMax + (dailyForecast[i].tempMax >= 0 ? .5 : -.5)); }
  int forecastWeatherId(int i) { return (int)dailyForecast[i].weatherId; }
  String forecastIcon(int i) { return dailyForecast[i].icon; }
  String getForecastMain(int i) { return dailyForecast[i].main; }

  // Methods to get Hourly forecast data
  time_t hourlyObservationTime(int i) { return hourlyForecast[i].observationTime; }
  int hourlyHourofDay(int i) {
    struct tm *timeinfo;
    timeinfo = localtime(&hourlyForecast[i].observationTime);
    return (int)timeinfo->tm_hour;
  }
  String hourlyHourofDayText(int i) {
    char buffer[20];
    struct tm *timeinfo;
    timeinfo = localtime(&hourlyForecast[i].observationTime);
    strftime(buffer, 20, "%I %p", timeinfo);
    return (String)buffer;
  }
  int hourlyTemp(int i) { return (int)(hourlyForecast[i].temp + (hourlyForecast[i].temp >= 0 ? .5 : -.5)); }
  int hourlyClouds(int i) { return (int)hourlyForecast[i].clouds; }
  int hourlyWeatherId(int i) { return (int)hourlyForecast[i].weatherId; }
  String hourlyIcon(int i) { return hourlyForecast[i].icon; }
  int hourlyPop(int i) { return (int)(hourlyForecast[i].pop * 100);}
  float hourlyPcpt(int i) { return hourlyForecast[i].pcpt;}


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
      _stream->print("timestamp: " + (String)ctime_r(&dailyForecast[i].observationTime, scratch));
      _stream->println("temp min: " + (String)dailyForecast[i].tempMin);
      _stream->println("temp max: " + (String)dailyForecast[i].tempMax);
      _stream->println("id: " + (String)dailyForecast[i].weatherId);
      _stream->println("main: " + (String)dailyForecast[i].main);
      _stream->println("decription: " + (String)dailyForecast[i].description);
      _stream->println("icon: " + (String)dailyForecast[i].icon);
      _stream->println();
    }

    for (int i = 0; i < 24; i++) {
      _stream->print("timestamp: " + (String)ctime_r(&hourlyForecast[i].observationTime, scratch));
      _stream->println("temp: " + (String)hourlyForecast[i].temp);
      _stream->println("id: " + (String)hourlyForecast[i].weatherId);
      _stream->println("clouds: " + (String)hourlyForecast[i].clouds);
      _stream->println("icon: " + (String)hourlyForecast[i].icon);
      _stream->println("pop: " + (String)hourlyForecast[i].pop);
      _stream->println("pcpt: " + (String)hourlyForecast[i].pcpt);
      _stream->println();
    }
  }
};

#endif /* WEATHER_H */