# ESP32 Ruuvi Nextion Weather Station

* Reads indoor and outdoor temperatures by listening for broadcasts from Ruuvi Bluetooth Tags
* Obtains current wind & humidity by calling Openweathermap Onecall API v3.0
* Displays weather by updating objects on Nextion Intellegent Display
* Displays 5-day forecast by updating Nextion objects using data from Openweathermap API

### Issues

Likely has memory leaks, may occasinally crash. 

### Libraries/Dependencies

*  Uses NimBLE (Bluetooth) to scan Ruuvi tags. 
*  Uses ArduinoJSON to manage JSON docs returned from Openweathermap API.

### Configuration

*  Edit settings-dist.h and rename to settings.h

### Nextion Configuration
Assumes Nextion device has at least the following objects/variables:

| Object Type | Object Name | Description |
|-------------|-------------|-------------|
| XFloat | indoorTemp | Indoor temperature (Ruuvi Tag) |
| XFloat | outdoorTemp | Outdoor Temperature (Ruuvi Tag) |
| Text | City | Text displayed as current location/city |
| Text | wxDescription | Current weather description |
| Picture | wxIcon | Current weather icon (Nextion picture object  number)
| Variable (int32) | windDirection | Wind direction in degrees |
| Number | windSpeed | Wind speed in MPH/KPH |
| Number | humidity | Humidity in Percent |
| Number | forecastMin1 | Min temp, Max temp, Date, Icon Number and description for next day forecast | |
| Number | forecastMax1 | |
| Text | dateTime1 | |
| Picture | forecastIcon1 | |
| Text | forecastTxt1 | |
| Number | forecastMin2 | Min temp, Max temp, Date, Icon Number and description for day 2 forecast |
| Number | forecastMax2 |
| Text | dateTime2 |
| Picture | forecastIcon2 |
| Text | forecastTxt2 |
| Number | forecastMin3 | Min temp, Max temp, Date, Icon Number and description for day 3 forecast |
| Number | forecastMax3 |
| Text | dateTime3 |
| Picture | forecastIcon3 |
| Text | forecastTxt3 |
| Number | forecastMin4 | Min temp, Max temp, Date, Icon Number and description for day 4 forecast |
| Number | forecastMax4 |
| Text | dateTime4 |
| Text | forecastTxt4 |
| Picture | forecastIcon4 |
| Number | forecastMin5 | Min temp, Max temp, Date, Icon Number and description for day 5 forecast |
| Number | forecastMax5 |
| Text | dateTime5 |
| Text | forecastTxt5 |
| Picture | forecastIcon5 |


