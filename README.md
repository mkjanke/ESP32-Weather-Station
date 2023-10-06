# ESP32 Ruuvi Nextion Weather Station

* Reads indoor and outdoor temperatures by listening for broadcasts from Ruuvi Bluetooth Tags
* Obtains current wind & humidity by calling Openweathermap Onecall API v3.0
* Displays weather by updating objects on Nextion Intellegent Display
* Displays 5-day forecast by updating Nextion objects using data from Openweathermap API v3.0

### Issues

Likely has memory leaks, may occasinally crash. 

### Libraries/Dependencies

*  Uses NimBLE (Bluetooth) to scan Ruuvi tags. 
*  Uses ArduinoJSON to manage JSON docs returned from Openweathermap API.
*  Uses FastLED to blink LED on ESP32 M5Stamp Pico

### Configuration

*  Edit settings-dist.h and rename to settings.h

### Nextion Configuration
Assumes Nextion device has at least the following objects/variables:

| Object Type | Object Name | Description |
|-------------|-------------|-------------|
| XFloat | page0.indoorTemp | Indoor temperature (Ruuvi Tag) |
| XFloat | page0.outdoorTemp | Outdoor Temperature (Ruuvi Tag) |
| Text | page0.City | Text displayed as current location/city |
| Text | page0.wxDescription | Current weather description |
| Picture | page0.wxIcon | Current weather icon (Nextion picture object  number)
| Variable (int32) | page0.windDirection | Wind direction in degrees |
| Number | page0.windSpeed | Wind speed in MPH/KPH |
| Number | page0.humidity | Humidity in Percent |
| Text | page0.statusTxt.txt | Connection status text |
| Text | Setup.OutdoorStatus.txt | status text |
| Text | Setup.indoorStatus.txt | status text |
| Text | Setup.WiFiStatus.txt | status text |
| Text | Setup.Heartbeat.txt | status text |
| Number | page0.forecastMin1 | Min temp, Max temp, Date, Icon Number and description for next day forecast | |
| Number | page0.forecastMax1 | |
| Text | page0.dateTime1 | |
| Picture | page0.forecastIcon1 | |
| Text | page0.forecastTxt1 | |
| Number | page0.forecastMin2 | Min temp, Max temp, Date, Icon Number and description for day 2 forecast |
| Number | page0.forecastMax2 |
| Text | page0.dateTime2 |
| Picture | page0.forecastIcon2 |
| Text | page0.forecastTxt2 |
| Number | page0.forecastMin3 | Min temp, Max temp, Date, Icon Number and description for day 3 forecast |
| Number | page0.forecastMax3 |
| Text | page0.dateTime3 |
| Picture | page0.forecastIcon3 |
| Text | page0.forecastTxt3 |
| Number | page0.forecastMin4 | Min temp, Max temp, Date, Icon Number and description for day 4 forecast |
| Number | page0.forecastMax4 |
| Text | page0.dateTime4 |
| Text | page0.forecastTxt4 |
| Picture | page0.forecastIcon4 |
| Number | page0.forecastMin5 | Min temp, Max temp, Date, Icon Number and description for day 5 forecast |
| Number | page0.forecastMax5 |
| Text | page0.dateTime5 |
| Text | page0.forecastTxt5 |
| Picture | page0.forecastIcon5 |


