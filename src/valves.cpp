// valves.cpp

#include "valves.h"

#define between(num, min, max) (((min) <= (num)) && ((num) < (max)))

// void valves::putInEEPROM() { EEPROM.put(0, outputValveValues); }

// void valves::begin() { EEPROM.begin(); }

// void valves::getFromEEPROM() { EEPROM.get(eeIndent, outputValveValues); }

void valves::loop(uint16_t minsSinceMidnight, valveTimes vTimes,
                  bool shouldLog) {

  for (uint8_t thisValve = 0; thisValve < 4; thisValve++) {

    bool shouldRun = false;

    for (uint8_t thisTime = 0; thisTime < 4; thisTime++) {

      if (shouldLog)
        if (shouldRun) {
          Serial.println(F("valve already on"));
          break;
        }

      uint16_t thisStartTime = vTimes.startTimes[thisValve][thisTime];
      uint16_t thisStopTime = vTimes.stopTimes[thisValve][thisTime];

      if ((thisStartTime == 0) && (thisStopTime == 0)) {
        shouldRun = false;
      } else if (between(minsSinceMidnight, thisStartTime, thisStopTime)) {
        shouldRun = true;
      } else {
        shouldRun = false;
      }

      if (shouldLog) {
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
        Serial.println(shouldRun);
      }
      outputValveValues[thisValve] = shouldRun;
    }
  }
}
