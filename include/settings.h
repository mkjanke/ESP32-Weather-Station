#ifndef SETTINGS_H
#define SETTINGS_H

#define WIFI_SSID "IoT"               //WiFi SSID
#define WIFI_PASSWORD "WTR54G2222"    // Wifi Password
#define DEVICE_NAME "ESP-Weather"     // Network name of device

#define OW_API_KEY "44196d9808785b1893057154d5b1521a"  // OpenWeather API Key
#define OW_LAT 45                                      // Location (lat/lon)
#define OW_LON -93
#define OW_CITY "Saint Paul"                           // City Name

#define RUUVI_5_SERVICE_ID "6e400001-b5a3-f393-e0a9-e50e24dcca9e" // Service ID for Ruuvi v5 temperature sensor
#define RUUVI_INDOOR_TAG "Ruuvi CC37"                             // Ruuvi tag name (from BLE broadcast)
#define RUUVI_INDOOR_DESCRIPTION "Indoor Device"
#define RUUVI_OUTDOOR_TAG "Ruuvi 39C1"                             // Ruuvi tag name (from BLE broadcast)
#define RUUVI_OUTDOOR_DESCRIPTION "Outdoor Device"

// Nextion Serial configuration
#define NEXTION_SERIAL Serial1    // Nextion Device Serial port
#define NEXTION_BAUD 115200       // Baud as set in Nextion Program startup
#define RXDN 19                   // Nextion Device Serial port pins
#define TXDN 21

#endif  // SETTINGS_H