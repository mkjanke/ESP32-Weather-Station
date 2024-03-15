#ifndef RUUVI_H
#define RUUVI_H

/* Ruuvi related classes and code.

  RuuviTag Object - one instance per tag
  RuuviScan Object - one instance

  See:

    * [https://github.com/ruuvi/ruuvi-sensor-protocols/blob/master/dataformat_05.md]
    * [https://mybeacons.info/packetFormats.html#hiresX]
    * [https://github.com/PascalBod/ESPIDFRuuviTag/blob/master/main/ruuvi_tag.c]

  Listens for Ruuvi Service UUID, mfg. ID 0x0499 and mfg. data version 0x05.
  Ignores all other advertisments
*/

#include <vector>

#include "NimBLEDevice.h"
#include "settings.h"

class RuuviTag {
 private:
  std::string _tagname;
  std::string _description;
  float _temperature = 0;
  int _humidity = 0;
  int _pressure = 0;
  time_t _lastUpdate;

 public:
  RuuviTag(std::string name, std::string description) {
    _tagname = name;
    _description = description;
  }

  void setTemperature(float temp) {
    time(&_lastUpdate);
    _temperature = temp;
  }
  void setHumidity(int humidity) { _humidity = humidity; }
  void setPressure(int pressure) { _pressure = pressure; }

  std::string getName() { return _tagname; }
  std::string getDescription() { return _description; }
  int getTemperatureInC() { return (int)(_temperature + (_temperature >= 0 ? .5 : -.5)); }
  int getTemperatureInF() { return (int)((((9.0f / 5) * (double)_temperature) + 32) + (_temperature >= 0 ? .5 : -.5)); }
  int getHumidity() { return _humidity; }
  int getPressureInPascal() { return _pressure; }
  int getPressureInMmHg() { return (int)(((double)_pressure) / 133.3223684); }
  time_t lastUpdate() { return _lastUpdate; }
};

// Create Ruuvi objects, two-node vector to help reference objects
RuuviTag indoorTag((std::string)RUUVI_INDOOR_TAG, RUUVI_INDOOR_DESCRIPTION);
RuuviTag outdoorTag((std::string)RUUVI_OUTDOOR_TAG, RUUVI_OUTDOOR_DESCRIPTION);
std::vector<RuuviTag*> ruuviList = {&indoorTag, &outdoorTag};

// Callback when any BLE device advertisement is received
class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
    // Ruuvi v5 serivice UUID
    NimBLEUUID serviceUuid(RUUVI_5_SERVICE_ID);

    if (advertisedDevice->getServiceUUID() == serviceUuid) {
      // Serial.printf("Ruuvi Device: %s \n", advertisedDevice->toString().c_str());

      // Look for Ruuvi mfg. ID
      // Only interested in Ruuvi v5's
      if (((byte)advertisedDevice->getManufacturerData().data()[0] == 0x99) &&
          ((byte)advertisedDevice->getManufacturerData().data()[1] == 0x04)) {
        std::string output = advertisedDevice->getName() + " " + advertisedDevice->getAddress().toString() + " ";
        Serial.print(output.c_str());

        char* manufacturerdata =
            NimBLEUtils::buildHexData(NULL, (uint8_t*)advertisedDevice->getManufacturerData().data(),
                                      advertisedDevice->getManufacturerData().length());
        // Serial.println(manufacturerdata);
        if (manufacturerdata != NULL) {
          free(manufacturerdata);
        }
        uint8_t* MFRdata;
        float tempInC = 0;
        float humPct = 0;
        float atmPressure = 0;

        MFRdata = (uint8_t*)advertisedDevice->getManufacturerData().data();

        // Version 5 data format
        // https://mybeacons.info/packetFormats.html#hiresX
        //
        if (MFRdata[2] == 0x05) {
          if (!(MFRdata[3] == 0x80 && MFRdata[4] == 0x00)) {
            tempInC = ((float)((int16_t)((MFRdata[4] << 0) | (MFRdata[3]) << 8)) * .005);
          }
          if (!(MFRdata[6] == 0xff && MFRdata[5] == 0xff)) {
            humPct = ((float)((uint16_t)((MFRdata[6] << 0) | (MFRdata[5]) << 8))) / 400;
          }
          if (!(MFRdata[8] == 0xff && MFRdata[7] == 0xff)) {
            atmPressure = ((float)((uint16_t)((MFRdata[8] << 0) | (MFRdata[7]) << 8)) + 50000);
          }

          // Populate Ruuvi object(s) with data from advertisement
          for (auto element : ruuviList) {
            if (advertisedDevice->getName() == element->getName()) {
              element->setTemperature(tempInC);
              element->setHumidity(humPct);
              element->setPressure(atmPressure);
              Serial.print(": ");
              Serial.print(element->getDescription().c_str());
              Serial.printf(" Temperature (C): %d Temperature (F): %d Humidity(%): %d Atm Pressure: %d\n",
                            element->getTemperatureInC(), element->getTemperatureInF(), element->getHumidity(),
                            element->getPressureInMmHg());
            }
          }
        }
      }

    } else {
      // advertisedDevice->getScan()->erase(advertisedDevice->getAddress());
    }
  }
};

class RuuviScan {
  friend MyAdvertisedDeviceCallbacks;

 private:
 public:
  NimBLEScan* pBLEScan;

  void begin() {
    NimBLEDevice::init("");
    pBLEScan = NimBLEDevice::getScan();  // create new scan

    // Set the callback for when devices are discovered, include duplicates.
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
    pBLEScan->setActiveScan(true);  // Set active scanning, this will get more data from the advertiser.

    // Values taken from NimBLE examples
    pBLEScan->setInterval(97);   // How often the scan occurs / switches channels; in milliseconds,
    pBLEScan->setWindow(37);     // How long to scan during the interval; in milliseconds.
    pBLEScan->setMaxResults(0);  // do not store the scan results, use callback only.
  }

  // Start BLE scan, block until scanTime timeout.
  void startRuuviScan(int scanTime) {
    if (pBLEScan->isScanning() == false) {
      Serial.println("Starting Ruuvi Scan");
      NimBLEScanResults foundDevices = pBLEScan->start(scanTime, false);
      pBLEScan->clearResults();
    }
  }
};

#endif  // RUUVI_H