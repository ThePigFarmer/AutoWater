// valves.h

#ifndef VALVES_h
#define VALVES_h

#include <Arduino.h>
#include <EEPROM.h>

class valves {
public:
  uint8_t outputValveValues[4];

  uint16_t startTimes[4][4]{
      // valve 1
      {250, 0, 0, 0},

      // valve 2
      {253, 0, 0, 420},

      // valve 3
      {254, 130, 1, 0},

      // valve 4
      {255, 0, 0, 0},
  };

  uint16_t stopTimes[4][4]{
      // valve 1
      {440, 0, 0, 0},

      // valve 2
      {13, 0, 0, 425},

      // valve 3
      {123, 132, 1438, 0},

      // valve 4
      {0, 0, 0, 0},
  };

  void putInEEPROM();
  void getFromEEPROM();
  void begin();
  void loop(uint16_t minsSinceMidnight);

  uint16_t eeIndent = 0;
};

#endif
