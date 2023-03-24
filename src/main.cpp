#include <Arduino.h>
#include <BtButton.h>
#include <DS3231.h>
#include <Wire.h>

#include "config.h"
#include "valves.h"

BtButton bnt(BUTTON_PIN);
DS3231 rtc;
Time t;
valves v;

uint8_t valvePins[4] = {2, 3, 4, 5};


uint32_t prevMillis;
const uint16_t timer1 = 1000;

uint16_t minutesSinceMidnight = 2;

void setup() {
  Serial.begin(MONITOR_SPEED);
  Wire.begin(); // for DS3231
  Serial.print("Serial and I2C started\n");

  // loadValveData(); // not for testing
} // end setup

void loop() {
  t = rtc.getTime();
  // uint16_t minutesSinceMidnight = t.hour * 60 + t.min;
   

  if ((millis() - prevMillis) > timer1) {
    prevMillis = millis();
    

    v.loop(minutesSinceMidnight);
  Serial.println("");
   
  for (uint8_t i = 0; i < 4; i++){
      digitalWrite(valvePins[i], v.outputValveValues[i]);
  
  Serial.println(v.outputValveValues[i]);
  }
    
  } // end timed loop

  bnt.read();

  if (bnt.changed()) {
    // press for eeprom write
    if (bnt.isPressed()) {
      // v.putInEEPROM();
      Serial.println("Saved valves to EEPROM");
    }
  }
} // end main loop
