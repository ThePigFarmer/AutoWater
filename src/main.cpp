#define MODE_IDLE 0
#define MODE_CONFIG 1

#include <Arduino.h>
#include <ArduinoJson.h>
#include <BtButton.h>
#include <DS3231.h>
#include <Wire.h>
#include <string.h>

#include "config.h"
#include "json_comm.h"
#include "timeCalc.h"
#include "valves.h"

BtButton bnt(BUTTON_PIN);
DS3231 rtc;
Time t;
valves v;

bool programMode = MODE_IDLE;

uint8_t valvePins[4] = {2, 3, 4, 5};

uint32_t prevMillis;
// sconst uint16_t timer1 = 1000;

void setup()
{
  Serial.begin(MONITOR_SPEED);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(11, INPUT_PULLUP);

  Serial.setTimeout(30000);

  Wire.begin(); // for DS3231

  Serial.print("Serial and I2C started\n");

  // loadValveData(); // not for testing
} // end setup

void loop()
{
  static StaticJsonDocument<192> json_doc;

  const auto deser_err = deserializeJson(json_doc, Serial);
  if (deser_err) {
    Serial.print(F("Failed to deserialize, reason: \""));
    Serial.print(deser_err.c_str());
    Serial.println('"');
  }
  else {
    Serial.print(F("Recevied valid json document with "));
    Serial.print(json_doc.size());
    Serial.println(F(" elements."));
    Serial.println(F("Pretty printed back at you:"));
    serializeJsonPretty(json_doc, Serial);
    Serial.println();

    // parse json data here

    // target
    char target[9];
    strcpy(target, json_doc[(char *)"target"]);

    // which valve (valveNumber)
    uint8_t valveNumber;
    valveNumber = json_doc[(uint8_t *)"valveNumber"];

    if (strcmp(target, "startCfg") == 0) {
      programMode = MODE_CONFIG;
    }

    // if ready for config
    if (programMode == MODE_CONFIG) {

      JsonArray startTimes = json_doc["startTimes"];
      /*const char *startTimes_0 = startTimes[0]; // "23:59"
      const char *startTimes_1 = startTimes[1]; // "23:59"
      const char *startTimes_2 = startTimes[2]; // "23:59"
      const char *startTimes_3 = startTimes[3]; // "23:59"*/

      JsonArray stopTimes = json_doc["stopTimes"];
      /*const char *stopTimes_0 = stopTimes[0]; // "23:59"
      const char *stopTimes_1 = stopTimes[1]; // "23:59"
      const char *stopTimes_2 = stopTimes[2]; // "23:59"
      const char *stopTimes_3 = stopTimes[3]; // "23:59*/

      for (uint8_t thisTimeInDay = 0; thisTimeInDay < MAX_TIMES_IN_DAY; thisTimeInDay++) {
        v.startTimes[valveNumber][thisTimeInDay] = computeMinutesSinceMidnight(startTimes[thisTimeInDay]);
        v.stopTimes[valveNumber][thisTimeInDay] = computeMinutesSinceMidnight(stopTimes[thisTimeInDay]);
      }

  

    }
  }

  t = rtc.getTime();
  uint16_t minutesSinceMidnight = t.hour * 60 + t.min;

  /* if ((millis() - prevMillis) > timer1) {*/
  if (!digitalRead(11)) {
    prevMillis = millis();

    v.loop(minutesSinceMidnight);
    Serial.println("");

    for (uint8_t i = 0; i < 4; i++) {
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
