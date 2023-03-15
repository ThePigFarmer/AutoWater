#define between(x, a, b) (((a) <= (x)) && ((x) <= (b)))

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <BtButton.h>
#include <DS3231.h>

#include "config.h"
#include "valves.h"

BtButton bnt(BUTTON_PIN);
DS3231 rtc;
Time t;

valves v;

uint32_t prevMillis;
const uint16_t timer1 = 1000;

void setup()
{
    Serial.begin(MONITOR_SPEED);
    Wire.begin(); // for DS3231
    Serial.print("Serial and I2C started\n");

    // loadValveData(); // not for testing
} // end setup

void loop()
{
    if ((millis() - prevMillis) > timer1)
    {
        prevMillis = millis();

        t = rtc.getTime(); // read DS3231 registers at once

        uint16_t minutesSinceMidnight = t.hour * 60 + t.min;

        // runValves(minutesSinceMidnight);
        Serial.println(minutesSinceMidnight);

        Serial.println(F("\n"));
    } // end timed loop

    bnt.read();

    if (bnt.changed())
    {
        // press for eeprom write
        if (bnt.isPressed())
        {
            // saveValveData();
            Serial.println(F("Saved valves to EEPROM"));
        }
    }
} // end main loop