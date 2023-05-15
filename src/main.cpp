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

uint16_t timeStrToMinsSinceMidnight(char timeStr[5]);

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

    Serial.println(F("making Jsonarrays..."));
    JsonArray startTimes = jsonDoc["startTimes"];
    // const char *startTimes_0 = startTimes[0]; // "23:59"
    // const char *startTimes_1 = startTimes[1]; // "23:59"
    // const char *startTimes_2 = startTimes[2]; // "23:59"
    // const char *startTimes_3 = startTimes[3]; // "23:59"

    JsonArray stopTimes = jsonDoc["stopTimes"];
    // const char *stopTimes_0 = stopTimes[0]; // "23:59"
    // const char *stopTimes_1 = stopTimes[1]; // "23:59"
    // const char *stopTimes_2 = stopTimes[2]; // "23:59"
    // const char *stopTimes_3 = stopTimes[3]; // "23:59"

    Serial.println(F("done"));
    Serial.println(F("now starting for loop"));
    Serial.println("");

    for (uint8_t i = 0; i < 4; i++) {
      char thisStartTime[5];
      strcpy(thisStartTime, startTimes[i]);
      char thisStopTime[5];
      strcpy(thisStopTime, stopTimes[i]);
      v.startTimes[valveID][i] = timeStrToMinsSinceMidnight(thisStartTime);
      v.stopTimes[valveID][i] = timeStrToMinsSinceMidnight(thisStopTime);
      Serial.print(F("start time: "));
      Serial.println(timeStrToMinsSinceMidnight(thisStartTime));
      Serial.print(F("stop time: "));
      Serial.println(timeStrToMinsSinceMidnight(thisStopTime));
    }
    Serial.print(F("valve set: "));
    Serial.println(valveID);
  }

  t = rtc.getTime();
  uint16_t minsSinceMidnight = t.hour * 60 + t.min;

  if ((millis() - prevMillis) > timer1) {
    // if (!digitalRead(11)) {
    prevMillis = millis();

    v.loop(minsSinceMidnight);
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

uint16_t timeStrToMinsSinceMidnight(char timeStr[5])
{
  // there must be a better way to do this. however, this works (hopefully)
  uint8_t hour[3] = {uint8_t(timeStr[0] - '0'), uint8_t(timeStr[1] - '0')};
  uint8_t min[3] = {uint8_t(timeStr[3] - '0'), uint8_t(timeStr[4] - '0')};

  hour[2] = hour[0] * 10 + hour[1];
  min[2] = min[0] * 10 + min[1];
  return uint16_t(hour[2] * 60 + min[2]);
}
