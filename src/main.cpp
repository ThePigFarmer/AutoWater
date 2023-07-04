#define between(num, min, max) (((min) <= (num)) && ((num) < (max)))

#define HTTP_PORT 80

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <RTClib.h>
#include <WiFi.h>

#include "valves.h"
valveTimes vTimes;
valves v;

RTC_DS3231 rtc;

const char *ssid = "CW Wifi";
const char *password = "Thewedels";

char *minsSinceMidnightToTimestr(uint16_t minsSinceMidnight);

void initValvePins();
void initWiFi();
void initRTC();

uint32_t prevMillis1, prevMillis2; // for timed loop

uint16_t timeStrToMinsSinceMidnight(char timeStr[6]);

uint8_t cntr;

uint8_t valvePins[4] = {4, 18, 19, 23};

void setup() {
  Serial.begin(115200);
  initValvePins();
  initWiFi();
  initRTC();
}

void loop() {

  if ((millis() - prevMillis1) > 1000) {
    prevMillis1 = millis();

    DateTime now = rtc.now();

    char timestr[9] = "hh:mm:ss";
    Serial.println(now.toString(timestr));

    uint16_t minsSinceMidnight = now.hour() * 60 + now.minute();

    if ((millis() - prevMillis2) > 5000) {
      prevMillis2 = millis();

      Serial.println(F("starting valve loop"));
      v.loop(minsSinceMidnight, vTimes);
      Serial.println(F("exit valve loop"));

      Serial.print(F("io: { "));
      for (uint8_t i = 0; i < 4; i++) {
        Serial.printf("(pin: %u, on: %u) ", valvePins[i],
                      v.outputValveValues[i]);
        digitalWrite(valvePins[i], v.outputValveValues[i]);
      }
      Serial.println(F("}"));
    }
  }
}

// ----------------------------------------------------------------------------
// Connecting to the WiFi network
// ----------------------------------------------------------------------------

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Trying to connect [%s] ", WiFi.macAddress().c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
}

void initRTC() {
  if (!rtc.begin()) {
    Serial.println("can't find rtc");
    Serial.flush();
    while (true)
      delay(1000);
  }

  /*if (rtc.lostPower()) {
      Serial.println("rtc lost power, setting time...");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    } */
}

char *minsSinceMidnightToTimestr(uint16_t minsSinceMidnight) {

  uint8_t mm = minsSinceMidnight % 60;
  uint8_t hh = (minsSinceMidnight - mm) / 60;

  static char timestr[9] = "hh:mm";
  sprintf(timestr, "%02u:%02u", hh, mm);
  return timestr;
}

void initValvePins() {
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(valvePins[i], OUTPUT);
  }
}
