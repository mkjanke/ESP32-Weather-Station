#ifndef NEXTIONINTERFACE_H
#define NEXTIONINTERFACE_H

#include <Arduino.h>
#include "settings.h"

/// @brief Class to handle communication with Nextion device.
/// @param serial Serial port to which Nextion is attached
/// @param baud Baud rate to be used for communication with Nextion
class myNextionInterface {
 private:
  HardwareSerial* _serial;
  unsigned long _baud;
  const char _cmdTerminator[3] = {'\xFF', '\xFF', '\xFF'};
  
  // Avoid smashing Nextion with overlapping reads and writes
  SemaphoreHandle_t _xSerialWriteSemaphore =  NULL;
  SemaphoreHandle_t _xSerialReadSemaphore = NULL;


 public:
  myNextionInterface(HardwareSerial&, unsigned long);

  bool begin();
  void flushReads();

  bool writeNum(const String&, uint32_t);
  bool writeStr(const String&, const String&); 
  bool writeCmd(const String&);

  bool setRTC(const tm);

  int listen(std::string&, uint8_t);
};

#endif  // NEXTIONINTERFACE_H