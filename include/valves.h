// valves.h

#ifndef VALVES_h
#define VALVES_h

#include <Arduino.h>
#include <EEPROM.h>

class valveTimes {

public:
  uint16_t startTimes[4][4]{
      // valve 1
      {0, 4, 0, 0},

      // valve 2
      {1, 5, 0, 0},

      // valve 3
      {2, 6, 0, 0},

      // valve 4
      {3, 7, 0, 0},
  };

  uint16_t stopTimes[4][4]{
      // valve 1
      {1, 5, 0, 0},

      // valve 2
      {2, 6, 0, 0},

      // valve 3
      {3, 7, 0, 0},

      // valve 4
      {4, 8, 0, 0},
  };
};

class valves {
public:
  uint8_t outputValveValues[4] = {0, 0, 0, 0};

  void putInEEPROM();
  void getFromEEPROM();
  void begin();
  void loop(uint16_t minsSinceMidnight, valveTimes vTimes);

  uint16_t eeIndent = 0;
};

#endif
