#define between(x, a, b) (((a) <= (x)) && ((x) <= (b)))

#include <Arduino.h>
#include <Wire.h>
#include <BtButton.h>

#include "config.h"
#include "valves.h"
#include "timeCalc.h"

BtButton bnt(BUTTON_PIN);

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

        // runValves(minutesSinceMidnight);
        Serial.println(minutesSinceMidnight());

        Serial.println(F("\n"));
    } // end timed loop

    bnt.read();

    if (bnt.changed())
    {
        // press for eeprom write
        if (bnt.isPressed())
        {
            // v.putInEEPROM();
            Serial.println(F("Saved valves to EEPROM"));
        }
    }
} // end main loop