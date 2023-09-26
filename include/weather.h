#ifndef WEATHER_H
#define WEATHER_H
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "settings.h"
#include "time.h"

/*----------------------------------------------------------------
Wrapper for OpenWeather API call (current weather, version 2.5)

Example:

curl "http://api.openweathermap.org/data/2.5/weather?id=5045360&appid=xxxxyourapikeyxxx"

Open Weather API JSON return format (St. Paul MN, 5045360):
{
  "coord": {
    "lon": -93.0933,
    "lat": 44.9444
  },
  "weather": [
    {
      "id": 802,
      "main": "Clouds",
      "description": "scattered clouds",
      "icon": "03d"
    }
  ],
  "base": "stations",
  "main": {
    "temp": 64.44,
    "feels_like": 63.27,
    "temp_min": 61.63,
    "temp_max": 69.66,
    "pressure": 1020,
    "humidity": 57
  },
  "visibility": 10000,
  "wind": {
    "speed": 11.5,
    "deg": 330
  },
  "clouds": {
    "all": 40
  },
  "dt": 1694550441,
  "sys": {
    "type": 1,
    "id": 5934,
    "country": "US",
    "sunrise": 1694519241,
    "sunset": 1694565024
  },
  "timezone": -18000,
  "id": 5045360,
  "name": "Saint Paul",
  "cod": 200
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
  uint16_t visibility;     // visibility: 10000,
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
  HTTPClient http;
  CurrentWeather weatherNow;        // Current Weather via API 3.0
  ForecastWeather forecast[8];      // Forecast - Eight days forcast availabe in API 3.0
  StaticJsonDocument<768> filter;   // ArduinoJSON Filter Document

  String currentWeatherHost = "http://api.openweathermap.org/data/3.0/onecall?appid=" + (String)OW_API_KEY +
                              "&lat=" + OW_LAT + "&lon=" + OW_LON + "&units=imperial";

 public:
  TaskHandle_t xhandlegetWeatherHandle = NULL;

  owmWeather() {
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
  void updateCurrentWeather() {
    http.useHTTP10(true);
    http.begin(currentWeatherHost);
    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

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
  }

  String getCurrentWeatherDescription() { return weatherNow.description; }
  int getCurrentWeatherOutdoorTemp() { return (int)weatherNow.temp; }
  int getCurrentWeatherHumidity() { return (int)weatherNow.humidity; }
  int getCurrentWeatherPressure() { return (int)weatherNow.pressure; }
  String getCurrentWeatherIcon() { return weatherNow.icon; }
  int getCurrentWeatherWindSpeed() { return (int)weatherNow.windSpeed; }
  int getCurrentWeatherWindDirection() { return (int)weatherNow.windDeg; }
  String getCurrentWeatherCityName() { return weatherNow.cityName; }

  time_t getForecastObservationTime(int i) { return forecast[i].observationTime; }
  String getForecastObservationDayofWeek(int i) {
    char buffer[20];
    struct tm *timeinfo;
    timeinfo = localtime(&forecast[i].observationTime);
    strftime(buffer, 20, "%a %d", timeinfo);
    return (String)buffer;
  }
  String getForecastDescription(int i) { return forecast[i].description; }
  int getForecastTempMin(int i) { return (int)forecast[i].tempMin; }
  int getForecastTempMax(int i) { return (int)forecast[i].tempMax; }
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
    _stream->println("city : " + weatherNow.cityName);
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