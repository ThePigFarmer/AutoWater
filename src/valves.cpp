#include "valves.h"

#define between(num, min, max) (((min) <= (num)) && ((num) <= (max)))

void valves::putInEEPROM() { EEPROM.put(0, times); }

void valves::begin() { EEPROM.begin(); }

void valves::getFromEEPROM() { EEPROM.get(eeIndent, times); }

uint8_t valves::run(uint16_t minsSinceMidnight) {
  uint8_t x[4];

  for (uint8_t thisValve = 0; thisValve < 4; thisValve++) {
    for (uint8_t thisTime = 0; thisTime < 4; thisTime++) {
      uint8_t thisStartTime = times[thisValve][0][thisTime];
      uint8_t thisStopTime = times[thisValve][1][thisTime];

      if (between(minsSinceMidnight, thisStartTime, thisStopTime)) {
        x[thisValve] = 1;
        break;
      } else {
        x[thisValve] = 0;
      }
    }
  }

  return x;
}
