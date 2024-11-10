#ifndef SETTINGS_H
#define SETTINGS_H

// Rename this file to settings.h
// Adjust WiFi, API, Nextion and Ruuvi #defines

#define WIFI_SSID "MY SSID"          // WiFi SSID
#define WIFI_PASSWORD "MY Password"  // Wifi Password
#define DEVICE_NAME "ESP-Weather"    // Network name of device

#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.nist.gov"
#define TZ_STRING "CST6CDT,M3.2.0,M11.1.0"  // Central time, America/Chicago

#define OW_SCAN_TIME 3                        // OpenWeather scan period (minutes)
#define OW_API_KEY "My Openweather API KEY"   // OpenWeather API Key
#define OW_LAT 45                             // Location (lat/lon)
#define OW_LON -92
#define OW_CITY "Nowhere USA"  // City Name

#define HEARTBEAT_INTERVAL_MILLIS 30000                            // Milliseconds between Ruuvi temp display updates
#define RUUVI_SCAN_TIME 10                                         //RUUVI tag scan time (seconds)  
#define RUUVI_5_SERVICE_ID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"  // Service ID for Ruuvi v5 temperature sensor
#define RUUVI_INDOOR_TAG "Ruuvi XXX"                               // Ruuvi tag name (from BLE broadcast) Set to name of your device
#define RUUVI_INDOOR_DESCRIPTION "Indoor Device"
#define RUUVI_OUTDOOR_TAG "Ruuvi YYY"                              // Ruuvi tag name (from BLE broadcast) Set to name of your device
#define RUUVI_OUTDOOR_DESCRIPTION "Outdoor Device"

// Nextion Serial configuration
#define NEXTION_SERIAL Serial1  // Nextion Device Serial port
#define NEXTION_BAUD 115200     // Baud as set in Nextion Program startup
#define RXDN 19                 // Nextion Device Serial port pins
#define TXDN 21

#endif  // SETTINGS_H