#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define between(num, min, max) (((min) <= (num)) && ((num) <= (max)))

#define HTTP_PORT 80

#include "config.h"
#include "valves.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <BtButton.h>
#include <ESPAsyncWebServer.h>
#include <RTClib.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <string.h>

AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");
BtButton bnt(BUTTON_PIN);
RTC_DS3231 rtc;
valveTimes vTimes;
valves v;

const char *WIFI_SSID = "CW Wifi:";
const char *WIFI_PASS = "Thewedels";

uint16_t timeStrToMinsSinceMidnight(char timeStr[6]);
char *minsSinceMidnightToTimeStr(uint16_t minsSinceMidnight);

uint8_t valvePins[4] = {4, 18, 19, 21};

uint32_t prevMillis;
const uint16_t timer1 = 1000;

void initSPIFFS();
void initWebServer();
void initWebSocket();
void initWiFi();
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len);

// -----------------------------------------------------------------------------
// main initialization
// -----------------------------------------------------------------------------

void setup()
{
  Serial.begin(MONITOR_SPEED);
  Serial.setTimeout(30000);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(11, INPUT_PULLUP);

  rtc.begin();

  Serial.println("Serial and rtc started");

  initSPIFFS();
  initWiFi();
  initWebServer();
  initWebSocket();

  Serial.println(F("setup done"));

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
    uint16_t mins = jsonDoc["min"];

    Serial.println(minsSinceMidnightToTimeStr(mins));

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

      vTimes.startTimes[valveID][i] = timeStrToMinsSinceMidnight(thisStartTime);

      vTimes.stopTimes[valveID][i] = timeStrToMinsSinceMidnight(thisStopTime);
    }
    // Serial.print(F("valve set: "));
    // Serial.println(valveID);
  }
  // end of json
  // -------------------------------------------------------------------------------------
  //
  //
  ws.cleanupClients();

  DateTime now = rtc.now();

  uint16_t minsSinceMidnight = now.hour() * 60 + now.minute();

  // timed loop
  if ((millis() - prevMillis) > timer1) {
    prevMillis = millis();

    Serial.println(F("now in valve loop"));
    v.loop(minsSinceMidnight, vTimes);
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

// -----------------------------------------------------------------------------
// TODO convert minutes since midnight to time string
// -----------------------------------------------------------------------------

char *minsSinceMidnightToTimeStr(uint16_t minsSinceMidnight)
{
  uint8_t min = minsSinceMidnight % 60;
  uint8_t hour = (minsSinceMidnight - min) / 60;

  static char timeStr[6];
  snprintf(timeStr, 6, "%02d:%02d", hour, min);

  return (char *)&timeStr;
}

// -----------------------------------------------------------------------------
// SPIFFS initalization
// -----------------------------------------------------------------------------

void initSPIFFS()
{
  if (!SPIFFS.begin()) {
    Serial.println(F("can't mount SPIFFS volumn"));
    while (1)
      digitalWrite(LED_BUILTIN, millis() % 200 < 50 ? 1 : 0);
  }
}

// -----------------------------------------------------------------------------
// WebServer intalization
// -----------------------------------------------------------------------------

void initWebServer()
{
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.begin();
}

// -----------------------------------------------------------------------------
// WebSocket initalization
// -----------------------------------------------------------------------------

void initWebSocket() { server.addHandler(&ws); }

// on event

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len)
{

  switch (type) {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(),
                  client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}



                                                                  // -----------------------------------------------------------------------------
                                                                  // WiFi
                                                                  // initalization
                                                                  // -----------------------------------------------------------------------------

                                                                  void
                                                                  initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("trying to connect [%s] ", WiFi.macAddress().c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
}
