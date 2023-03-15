#include "valves.h"

void valves::putInEEPROM()
{
    EEPROM.put(0, times);
}

void valves::begin()
{
    EEPROM.begin();
}

void valves::getFromEEPROM()
{
    EEPROM.get(eeIndent, times);
}

void run(uint16_t minsSinceMidnight)
{
    for (uint8_t thisValve = 0; thisValve < 4; thisValve++)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
        }
    }
}