#pragma once

#include <Arduino.h>
#include <EEPROM.h>


class valves
{
public:
    uint16_t times[4][2][4]
    {
        // valve 1
        {
            // start times
            {1, 5, 9, 13},

            // stop times
            {2, 6, 10, 14},
        },

        // valve 2
        {
            // start times
            {2, 6, 10, 14},

            // stop times
            {3, 7, 11, 15},
        },

        // valve 3
        {
            // start times
            {3, 7, 11, 15},

            // stop times
            {4, 8, 12, 16},
        },

        // valve 4
        {
            // start times
            {4, 8, 12, 16},

            // stop times
            {5, 9, 13, 17},
        },
    };

    void putInEEPROM();
    void getFromEEPROM();
    void begin();
    void run(uint16_t minsSinceMidnight);

    uint16_t eeIndent = 0;
};
