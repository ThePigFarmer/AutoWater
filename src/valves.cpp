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