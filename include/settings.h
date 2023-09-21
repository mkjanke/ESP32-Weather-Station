#ifndef SETTINGS_H
#define SETTINGS_H

#define WIFI_SSID "IoT"
#define WIFI_PASSWORD "WTR54G2222"
#define DEVICE_NAME "ESP-Weather"

#define OW_API_KEY "593a37132264f8670b2075f2925b6365"
#define OW_CITY_CODE "5045360"

#define HEARTBEAT 1000L         // Sensor and WiFi loop delay (ms)

#define RUUVI_5_SERVICE_ID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define RUUVI_INDOOR_TAG "Ruuvi CC37"
#define RUUVI_INDOOR_DESCRIPTION "Indoor Device"
#define RUUVI_OUTDOOR_TAG "Ruuvi 39C1"
#define RUUVI_OUTDOOR_DESCRIPTION "Outdoor Device"

// Nextion Serial configuration
#define NEXTION_SERIAL Serial1
#define NEXTION_BAUD 115200
#define RXDN 19
#define TXDN 21


#endif //SETTINGS_H