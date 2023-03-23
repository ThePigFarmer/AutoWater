#pragma once

#include <Arduino.h>
#include <EEPROM.h>

class valves {
public:
  uint8_t outputValveValues[4];

  uint16_t startTimes[4][4]{
      // valve 1
      {3, 5, 9, 13},

      // valve 2
      {2, 6, 10, 14},

      // valve 3
      {3, 7, 11, 15},

      // valve 4
      {4, 8, 12, 16},
  };

  uint16_t stopTimes[4][4]{
      // valve 1
      {3, 5, 9, 13},

      // valve 2
      {2, 6, 10, 14},

      // valve 3
      {3, 7, 11, 15},

      // valve 4
      {4, 8, 12, 16},
  };

  void putInEEPROM();
  void getFromEEPROM();
  void begin();
  void run(uint16_t minsSinceMidnight);

  uint16_t eeIndent = 0;
};
