// valves.cpp

#include "valves.h"

#define between(num, min, max) (((min) <= (num)) && ((num) <= (max)))

// void valves::putInEEPROM() { EEPROM.put(0, outputValveValues); }

// void valves::begin() { EEPROM.begin(); }

// void valves::getFromEEPROM() { EEPROM.get(eeIndent, outputValveValues); }

void valves::loop(uint16_t minsSinceMidnight)
{
  for (uint8_t thisValve = 0; thisValve < 4; thisValve++) {

    for (uint8_t thisTime = 0; thisTime < 4; thisTime++) {

      uint16_t thisStartTime = startTimes[thisValve][thisTime];
      uint16_t thisStopTime = stopTimes[thisValve][thisTime];

      bool isBetween = between(minsSinceMidnight, thisStartTime, thisStopTime);

      Serial.print("on valve #");
      Serial.print(thisValve);
      Serial.print("  time of day #");
      Serial.print(thisTime);
      Serial.print("   start time: ");
      Serial.print(thisStartTime);
      Serial.print("   stop time: ");
      Serial.print(thisStopTime);
      Serial.print("   mins since midnite: ");
      Serial.print(minsSinceMidnight);
      Serial.print("   should run: ");

      if (isBetween) {
        outputValveValues[thisValve] = 1;
        Serial.println("true");
        break;
      }
      else {
        outputValveValues[thisValve] = 0;
        Serial.println("false");
      }
    }
  }
}
