#include "valves.h"

#define between(num, min, max) (((min) <= (num)) && ((num) <= (max)))

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

bool run(uint16_t minsSinceMidnight)
{
    bool x[4];

    for (uint8_t thisValve = 0; thisValve < 4; thisValve++)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            if (between(minsSinceMidnight, times[thisValve][0][i], times[thisValve][1][i]))
            {
                x[thisValve] = true;
                break;
            }
            else
            {
                x[thisValve] = false;
            }
        }
    }
}
