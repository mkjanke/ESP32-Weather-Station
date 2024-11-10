#include "nextionInterface.h"

/// @brief Class to handle communication with Nextion device.
/// @param serial Serial port to which Nextion is attached
/// @param baud Baud rate to be used for communication with Nextion
myNextionInterface::myNextionInterface(HardwareSerial& serial, unsigned long baud) {
  _serial = &serial;
  _baud = baud;

  // Initialize semaphores
  _xSerialWriteSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(_xSerialWriteSemaphore);
  _xSerialReadSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(_xSerialReadSemaphore);
}

/// @brief Initialize Nextion Interface
///        Flush serial interface, reset Nextion display
/// @return true
bool myNextionInterface::begin() {
  vTaskDelay(100 / portTICK_PERIOD_MS);
  _serial->begin(_baud, SERIAL_8N1, RXDN, TXDN);

  vTaskDelay(400 / portTICK_PERIOD_MS);  // Pause for effect
  flushReads();

  // Reset Nextion Display
  vTaskDelay(100 / portTICK_PERIOD_MS);
  writeCmd("rest");
  return true;
}

/// @brief Read and throw away serial input until no bytes or timeout
void myNextionInterface::flushReads() {
  if (_xSerialReadSemaphore != NULL) {
    if (xSemaphoreTake(_xSerialReadSemaphore, 400 / portTICK_PERIOD_MS)) {
      unsigned long _timer = millis();
      while ((_serial->available() > 0) && (millis() - _timer) < 400L) {
        _serial->read();  // Start with clear serial port.
      }
      xSemaphoreGive(_xSerialReadSemaphore);
    }
  }
}

/// @brief Write Nextion formatted 'Number' to Nextion display objects 'val' property
/// @param _componentName Name of Nextion object/component
/// @param _val Number to write
/// @return Success or not
bool myNextionInterface::writeNum(const String& _componentName, uint32_t _val) {
  String _command = _componentName + "=" + _val;
  if (_xSerialWriteSemaphore != NULL) {
    if (xSemaphoreTake(_xSerialWriteSemaphore, 100 / portTICK_PERIOD_MS) == pdTRUE) {
      _serial->print(_command + _cmdTerminator);
      xSemaphoreGive(_xSerialWriteSemaphore);
      return true;
    }
  }
  return false;
}  // writeNum()

/// @brief Write Arduino String object to Nextion display objects 'txt' property
/// @param command Name of Nextion object/component
/// @param txt String to write
/// @return Success or not
bool myNextionInterface::writeStr(const String& _componentName, const String& txt) {
  String _command = _componentName + "=\"" + txt + "\"";
  if (_xSerialWriteSemaphore != NULL) {
    if (xSemaphoreTake(_xSerialWriteSemaphore, 100 / portTICK_PERIOD_MS) == pdTRUE) {
      _serial->print(_command + _cmdTerminator);
      xSemaphoreGive(_xSerialWriteSemaphore);
      return true;
    }
  }
  return false;
}  // writeStr()

/// @brief Write a generic Nextion command to the display
/// @param command Nextion command
/// @return Success or not
bool myNextionInterface::writeCmd(const String& command) {
  if (_xSerialWriteSemaphore != NULL) {
    if (xSemaphoreTake(_xSerialWriteSemaphore, 100 / portTICK_PERIOD_MS) == pdTRUE) {
      _serial->print(command + _cmdTerminator);
      xSemaphoreGive(_xSerialWriteSemaphore);
      return true;
    }
  }
  return false;
}  // writeCmd()

/// @brief Set Nextion Real Time Clock (RTC)
/// @param time tm struct containing time to set
/// @return true if RTC set successfully
bool myNextionInterface::setRTC(tm time) {
  if (writeNum((String) "rtc0", time.tm_year + 1900) && writeNum((String) "rtc1", time.tm_mon + 1) &&
      writeNum((String) "rtc2", time.tm_mday) && writeNum((String) "rtc3", time.tm_hour) &&
      writeNum((String) "rtc4", time.tm_min) && writeNum((String) "rtc5", time.tm_sec))
    return true;
  else
    return false;
}

/// @brief Listen for data from Nextion device
///        Call in a task or the loop() function periodically.
/// @param _nexBytes std::string in which to place bytes from Nextion
/// @param _size Max number of bytes to read
/// @return Number of bytes read or 'false' if no bytes read
int myNextionInterface::listen(std::string& _nexBytes, uint8_t _size) {
  if (_xSerialReadSemaphore != NULL) {
    if (xSemaphoreTake(_xSerialReadSemaphore, 100 / portTICK_PERIOD_MS)) {
      if (_serial->available() > 0) {
        int _byte_read = 0;
        uint8_t _terminatorCount = 0;  // Expect three '\xFF' bytes at end of each Nextion command
        unsigned long _timer = millis();

        while ((_nexBytes.length() < _size) && (_terminatorCount < 3) && (millis() - _timer) < 400L) {
          _byte_read = _serial->read();
          if (_byte_read != -1) {  // HardwareSerial::read returns -1 if no bytes available
            _nexBytes.push_back(_byte_read);
          }
          if (_byte_read == '\xFF') _terminatorCount++;
        }
        xSemaphoreGive(_xSerialReadSemaphore);
        return _nexBytes.length();
      }
      xSemaphoreGive(_xSerialReadSemaphore);
      return false;
    } else {
      return false;
    }
  } else
    return false;
}  // listen()
