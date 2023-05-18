#define MODE_IDLE 0
#define MODE_CONFIG 1

#define between(num, min, max) (((min) <= (num)) && ((num) <= (max)))

#include "config.h"
#include "valves.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <BtButton.h>
#include <DS3231.h>

BtButton bnt(BUTTON_PIN);
DS3231 rtc;
valves v;
Time t;

uint16_t timeStrToMinsSinceMidnight(char timeStr[6]);

bool programMode = MODE_IDLE;

#if defined(ESPRESSIF32)
uint8_t valvePins[4] = {16, 17, 18, 19};
#else
uint8_t valvePins[4] = {2, 3, 4, 5};
#endif

uint32_t prevMillis;
const uint16_t timer1 = 1000;

void setup()
{
  Serial.begin(MONITOR_SPEED);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(11, INPUT_PULLUP);

  Serial.setTimeout(30000);

  Wire.begin(); // for DS3231

  Serial.print("Serial and I2C started\n");

} // end setup

void loop()
{
  static StaticJsonDocument<256> jsonDoc;

  const auto deser_err = deserializeJson(jsonDoc, Serial);
  if (deser_err) {
    Serial.print(F("Failed to deserialize, reason: \""));
    Serial.print(deser_err.c_str());
    Serial.println('"');
  }
  else {
    Serial.print(F("Recevied valid json document with "));
    Serial.print(jsonDoc.size());
    Serial.println(F(" elements."));
    Serial.println(F("Pretty printed back at you:"));
    serializeJsonPretty(jsonDoc, Serial);
    Serial.println();

    // parse json data here ----------------------------------------------------

    const char *target = jsonDoc["target"];
    int8_t valveID = jsonDoc["valveID"] | -1;

    // should we config?
    if (!(strcmp(target, "cfg") == 0)) {
      Serial.println(F("no cfg"));
      return;
    }

    // do we have valve selected?
    if (valveID == -1) {
      Serial.println(F("invalid valve"));
      return;
    }

    JsonArray startTimes = jsonDoc["startTimes"];

    JsonArray stopTimes = jsonDoc["stopTimes"];

    for (uint8_t i = 0; i < 4; i++) {

      char thisStartTime[6];
      strcpy(thisStartTime, startTimes[i]);

      char thisStopTime[6];
      strcpy(thisStopTime, stopTimes[i]);

      Serial.print(F("doing start time: "));
      v.startTimes[valveID][i] = timeStrToMinsSinceMidnight(thisStartTime);
      Serial.println(thisStartTime);

      Serial.print(F("doing stop time"));
      v.stopTimes[valveID][i] = timeStrToMinsSinceMidnight(thisStopTime);
      Serial.println(thisStopTime);
    }
    // Serial.print(F("valve set: "));
    // Serial.println(valveID);
  }
  // end of json
  // -------------------------------------------------------------------------------------
  //
  //

  t = rtc.getTime(); // get time
  uint16_t minsSinceMidnight = t.hour * 60 + t.min;

  // timed loop
  if ((millis() - prevMillis) > timer1) {
    prevMillis = millis();

    Serial.println(F("now in valve loop"));
    v.loop(minsSinceMidnight);
    Serial.println(F("out of valve loop"));

    Serial.print(F("setting digital pin states: {"));
    for (uint8_t i = 0; i < 4; i++) {
      digitalWrite(valvePins[i], v.outputValveValues[i]);
      Serial.print(v.outputValveValues[i]);
      Serial.print(",");
    }
    Serial.println("}");

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

uint16_t timeStrToMinsSinceMidnight(char timeStr[6])
{
  // there must be a better way to do this. however, this works (hopefully)
  uint8_t hour = uint8_t(timeStr[0] - '0') * 10 + uint8_t(timeStr[1] - '0');
  uint8_t min = uint8_t(timeStr[3] - '0') * 10 + uint8_t(timeStr[4] - '0');

  return uint16_t(hour * 60 + min);
}
