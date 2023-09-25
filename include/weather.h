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
  float tempMin;           // "temp_min": 289.15,
  float tempMax;           // "temp_max": 292.15
  uint16_t visibility;     // visibility: 10000,
  float windSpeed;         // "wind": {"speed": 1.5},
  float windDeg;           // "wind": {deg: 226.505},
  uint8_t clouds;          // "clouds": {"all": 90},
  time_t observationTime;  // "dt": 1527015000,
  String country;          // "country": "CH",
  time_t sunrise;          // "sunrise": 1526960448,
  time_t sunset;           // "sunset": 1527015901
  String cityName;         // "name": "Zurich",
};

class owmWeather {
 private:
  WiFiClient client;
  HTTPClient http;
  CurrentWeather weatherNow;
  String currentWeatherHost = "http://api.openweathermap.org/data/2.5/weather?appid=" + (String)OW_API_KEY +
                   "&id=" + OW_CITY_CODE + "&units=imperial";
 public:  
  TaskHandle_t xhandlegetWeatherHandle = NULL;

  void getCurrentWeather(){
    http.useHTTP10(true);
    http.begin(currentWeatherHost);
    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      StaticJsonDocument<1000> doc;
      deserializeJson(doc, http.getStream());
      Serial.println("JSON DOC:");

      // Extract JSON to struct elements
      weatherNow.lon = doc["coord"]["lon"].as<float>();
      weatherNow.lat = doc["coord"]["lat"].as<float>();
      weatherNow.weatherId = doc["weather"]["id"].as<unsigned int>();
      weatherNow.main = doc["weather"][0]["main"].as<String>();
      weatherNow.description = doc["weather"][0]["description"].as<String>();
      weatherNow.icon = doc["weather"][0]["icon"].as<String>();
      weatherNow.temp = doc["main"]["temp"].as<float>();
      weatherNow.tempMin = doc["main"]["temp_min"].as<float>();
      weatherNow.tempMax = doc["main"]["temp_max"].as<float>();
      weatherNow.feelsLike = doc["main"]["feels_like"].as<float>();
      weatherNow.pressure = doc["main"]["pressure"].as<unsigned int>();
      weatherNow.humidity = doc["main"]["humidity"].as<unsigned int>();
      weatherNow.tempMax = doc["main"]["temp_max"].as<float>();
      weatherNow.windSpeed = doc["wind"]["speed"].as<float>();
      weatherNow.windDeg = doc["wind"]["deg"].as<float>();
      weatherNow.clouds = doc["clouds"]["all"].as<unsigned int>();
      weatherNow.observationTime = doc["dt"].as<time_t>();
      weatherNow.country = doc["sys"]["country"].as<String>();
      weatherNow.cityName = doc["name"].as<String>();
      weatherNow.sunrise = doc["sys"]["sunrise"].as<time_t>();
      weatherNow.sunset = doc["sys"]["sunset"].as<time_t>();
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }

  String getDescription() { return weatherNow.description; }
  int getOutdoorTemp() { return (int)weatherNow.temp; }
  int getHumidity() { return (int)weatherNow.humidity; }
  int getPressure(){ return (int)weatherNow.pressure; }
  String getIcon() { return weatherNow.icon; }
  int getWindSpeed() { return (int)weatherNow.windSpeed; }
  int getWindDirection() { return (int)weatherNow.windDeg; }
  String getCityName() { return weatherNow.cityName; }

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
    _stream->println("temp min: " + (String)weatherNow.tempMin);
    _stream->println("temp max: " + (String)weatherNow.tempMax);
    _stream->println("feelsLike : " + (String)weatherNow.feelsLike);
    _stream->println("pressure : " + (String)weatherNow.pressure);
    _stream->println("humidity : " + (String)weatherNow.humidity);
    _stream->println("wind speed : " + (String)weatherNow.windSpeed);
    _stream->println("wind dir : " + (String)weatherNow.windDeg);
    _stream->println("clouds : " + (String)weatherNow.clouds);
    _stream->println("timestamp : " + (String)ctime_r(&weatherNow.observationTime, scratch));
    _stream->println("country : " + weatherNow.country);
    _stream->println("city : " + weatherNow.cityName);
    _stream->print("sunrise : " + (String)ctime_r(&weatherNow.sunrise, scratch));
    _stream->println("sunset : " + (String)ctime_r(&weatherNow.sunset, scratch));
  }
};

#endif /* WEATHER_H */