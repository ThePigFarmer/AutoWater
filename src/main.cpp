#define between(num, min, max) (((min) <= (num)) && ((num) < (max)))

#define BAUD_RATE monitor_speed
#define VALVES_MAX_RUNS_IN_DAY 4
#define VALVES_MAX_NUM 4
#define HTTP_PORT 80

#include "valves.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <BtButton.h>
#include <ESPAsyncWebServer.h>
#include <RTClib.h>
#include <WiFi.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

BtButton bnt(34);
valveTimes vTimes;
valves v;

RTC_DS3231 rtc;

const char *ssid = "CW Wifi";
const char *password = "Thewedels";

char *minsSinceMidnightToTimestr(uint16_t minsSinceMidnight);

void doSerialLogging();
void initValvePins();
void initRTC();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void initWebServer();
void initWebSocket();
void initWiFi();
void notifyClients();
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len);
void sendValveTimesToClient(uint8_t valveID);

uint32_t prevMillis1, prevMillis2; // for timed loop

uint16_t timeStrToMinsSinceMidnight(char timeStr[6]);

uint8_t cntr;

uint8_t valvePins[4] = {4, 18, 19, 23};

struct Log {
  bool time;
  bool valves;
  bool wifi;
  bool json;
  bool valveLoop;
};
Log shouldLog{false, false, false, false, false};

void setup() {
  Serial.begin(BAUD_RATE);
  initValvePins();
  initWiFi();
  initRTC();
  initWebServer();
  initWebSocket();
}

void loop() {

  doSerialLogging();

  if ((millis() - prevMillis1) > 1000) {
    prevMillis1 = millis();

    DateTime now = rtc.now();

    if (shouldLog.time) {
      char timestr[9] = "hh:mm:ss";
      Serial.println(now.toString(timestr));
    }
    uint16_t minsSinceMidnight = now.hour() * 60 + now.minute();

    if ((millis() - prevMillis2) > 5000) {
      prevMillis2 = millis();

      v.loop(minsSinceMidnight, vTimes, shouldLog.valveLoop);

      for (uint8_t i = 0; i < VALVES_MAX_NUM; i++) {
        if (shouldLog.valves)
          Serial.printf("(pin: %u, on: %u) ", valvePins[i],
                        v.outputValveValues[i]);
        digitalWrite(valvePins[i], v.outputValveValues[i]);
      }
    }
  }

  // button stuff
  bnt.read();

  if (bnt.changed()) {
    if (bnt.isPressed()) {
      sendValveTimesToClient(0);
    }
  }
} // end main loop

// -----------------------------------------------------------------------------
// functions
// -----------------------------------------------------------------------------

char *minsSinceMidnightToTimestr(uint16_t minsSinceMidnight) {

  uint8_t mm = minsSinceMidnight % 60;
  uint8_t hh = (minsSinceMidnight - mm) / 60;

  static char timestr[9] = "hh:mm";
  sprintf(timestr, "%02u:%02u", hh, mm);
  return timestr;
}

uint16_t timeStrToMinsSinceMidnight(char timeStr[6]) {
  // there must be a better way to do this. however, this works (hopefully)
  uint8_t hour = uint8_t(timeStr[0] - '0') * 10 + uint8_t(timeStr[1] - '0');
  uint8_t min = uint8_t(timeStr[3] - '0') * 10 + uint8_t(timeStr[4] - '0');

  return uint16_t(hour * 60 + min);
}

void doSerialLogging() {
  if (Serial.available() == 1) {

    char inByte = Serial.read();

    switch (inByte) {
    case 'j':
      shouldLog.json = !shouldLog.json;
      break;
    case 't':
      shouldLog.time = !shouldLog.time;
      break;
    case 'v':
      shouldLog.valves = !shouldLog.valves;
      break;
    case 'l':
      shouldLog.valveLoop = !shouldLog.valveLoop;
      break;
    }
  } else
    return;
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len &&
      info->opcode == WS_TEXT) {

    const uint8_t size = JSON_OBJECT_SIZE(2);
    StaticJsonDocument<size> json;
    DeserializationError err = deserializeJson(json, data);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return;
    } else {
      if (shouldLog.json) {
        Serial.print(F("Recevied valid json document with "));
        Serial.print(json.size());
        Serial.println(F(" elements."));
        Serial.println(F("Pretty printed back at you:"));
        serializeJsonPretty(json, Serial);
        Serial.println("");
        // parse json somewhere here
      }

      const char *action = json["action"];

      Serial.printf("action: %s/n", action);

      if (strcmp(action, "getValveCfg") == 0) {

        int8_t valveID = json["valveID"] | -1;
        Serial.printf("valveID: %i/n", valveID);

        if ((valveID > -1) && (valveID < (VALVES_MAX_NUM))) {
          sendValveTimesToClient(valveID);
        } else {
          Serial.println("invalid valveID");
        }

      } else {
        Serial.println("action not getValveCfg");
      }
    }
    /*
    const char *action = json["action"];
    if (strcmp(action, "toggle") == 0) {
      led.on = !led.on;
      notifyClients();
    }
    */
  }
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

void initValvePins() {
  for (uint8_t i = 0; i < VALVES_MAX_NUM; i++) {
    pinMode(valvePins[i], OUTPUT);
  }
}

void initWebServer() { server.begin(); }

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

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

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {

  switch (type) {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(),
                  client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void sendValveTimesToClient(uint8_t valveID) {
  StaticJsonDocument<256> doc;
  doc["action"] = "cfg";
  doc["valveID"] = valveID;

  // stopTimes.add("23:59");

  JsonArray startTimes = doc.createNestedArray("startTimes");
  JsonArray stopTimes = doc.createNestedArray("stopTimes");

  for (uint8_t i = 0; i < VALVES_MAX_RUNS_IN_DAY; i++) {
    startTimes.add(minsSinceMidnightToTimestr(vTimes.startTimes[valveID][i]));
    stopTimes.add(minsSinceMidnightToTimestr(vTimes.stopTimes[valveID][i]));
  }

  char buffer[160];
  size_t len = serializeJson(doc, buffer);
  ws.textAll(buffer, len);
  Serial.printf("sent json: \n %.*s\n", (uint8_t)len, buffer);
}
