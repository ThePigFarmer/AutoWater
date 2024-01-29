#define between(num, min, max) (((min) <= (num)) && ((num) < (max)))

#define BAUD_RATE monitor_speed
#define VALVES_MAX_RUNS_IN_DAY 4
#define VALVES_MAX_NUM 4
#define HTTP_PORT 80

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <BtButton.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>
#include <RTClib.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <string.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

BtButton bnt(34);

IPAddress staticIP(10, 0, 0, 239);
IPAddress gateway(10, 0, 0, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(10, 0, 0, 254);

RTC_DS3231 rtc;

bool isValveValid(int8_t valveID);

const char *ssid = "CW Wifi";
const char *password = "Thewedels";

void initValvePins();
void initRTC();
void getValvesTimesFromEEPROM();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void initSPIFFS();
void initWebServer();
void initWebSocket();
void initWiFi();
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len);
void putValvesTimesInEEPROM();
void sendValveTimesToClients(uint8_t valveID);
void setValveTime(uint8_t valveID, uint8_t timeInDay, uint16_t calcedStartTime,
                  uint16_t calcedStopTime);
void valvesLoop(uint16_t minsSinceMidnight, bool shouldLog);

uint32_t prevMillis1, prevMillis2; // for timed loop

uint16_t timeStrToMinsSinceMidnight(char timeStr[6]);

struct ValveTimes {
  uint16_t start[4][4]{
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
  };

  uint16_t stop[4][4]{
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
  };
} valveTimes;

uint8_t valvePins[4] = {4, 18, 19, 23};
uint8_t valvesOutputValues[4];

void setup() {
  Serial.begin(BAUD_RATE);
  EEPROM.begin(130);
  initValvePins();
  initWiFi();
  initSPIFFS();
  initRTC();
  initWebServer();
  initWebSocket();

  getValvesTimesFromEEPROM();
}

void loop() {

  ws.cleanupClients();
  prevMillis1 = millis();

  DateTime now = rtc.now();

  char timestr[9] = "hh:mm:ss";
  sprintf(timestr, "%02u:%02u:%02u", now.hour(), now.minute(), now.second());

  uint16_t minsSinceMidnight = now.hour() * 60 + now.minute();

  if ((millis() - prevMillis2) > 5000) {
    prevMillis2 = millis();

    valvesLoop(minsSinceMidnight, false);

    for (uint8_t i = 0; i < VALVES_MAX_NUM; i++) {
      // Serial.printf("(pin: %u, on: %u\n) ", valvePins[i],
      // valvesOutputValues[i]);
      digitalWrite(valvePins[i], valvesOutputValues[i]);
    }
  }
  /*
    // button stuff
    bnt.read();

    if (bnt.changed()) {
      if (bnt.isPressed()) {
        sendRTCTimeToClients(timestr);
      }
    }
    */
} // end main loop

// -----------------------------------------------------------------------------
// functions
// -----------------------------------------------------------------------------

char *minsSinceMidnightToTimestr(uint16_t minsSinceMidnight) {

  uint8_t mm = minsSinceMidnight % 60;
  uint8_t hh = (minsSinceMidnight - mm) / 60;

  static char timestr[6] = "hh:mm";
  sprintf(timestr, "%02u:%02u", hh, mm);
  return timestr;
}

uint16_t timeStrToMinsSinceMidnight(char timestr[6]) {
  // there must be a better way to do this. however, this works (hopefully)
  uint8_t hour = uint8_t(timestr[0] - '0') * 10 + uint8_t(timestr[1] - '0');
  uint8_t min = uint8_t(timestr[3] - '0') * 10 + uint8_t(timestr[4] - '0');

  return uint16_t(hour * 60 + min);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len &&
      info->opcode == WS_TEXT) {

    const uint8_t size = JSON_OBJECT_SIZE(5);

    StaticJsonDocument<size> json;
    DeserializationError err = deserializeJson(json, data);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return;
    } else {
      /*if (shouldLog.json) {
        Serial.print(F("Recevied valid json document with "));
        Serial.print(json.size());
        Serial.println(F(" elements."));
        Serial.println(F("Pretty printed back at you:"));
        serializeJsonPretty(json, Serial);
        Serial.println("");*/
    }

    // parse json ------------------------------------------------------------

    const char *action = json["action"];
    Serial.printf("action: %s\n", action);

    // valveID (if any)
    uint8_t valveID = json["valveID"];
    Serial.printf("valveID: %i\n", valveID);

    bool valveIsValid = (valveID < VALVES_MAX_NUM) ? true : false;

    Serial.printf("is valve valid: %s\n", valveIsValid ? "true" : "false");

    // now for the if (json.isWhatever)
    if ((strcmp(action, "getValveCfg") == 0) && (valveIsValid)) {
      sendValveTimesToClients(valveID);
      return;
    }

    if ((strcmp(action, "setValveCfg") == 0) && valveIsValid) {
      uint8_t timeInDay = json["timeInDay"];
      Serial.printf("time in day: %u", timeInDay);

      char strConversionBuffer[6]; // for converting const char* to char*. if
                                   // someone knows of a better way, please
                                   // let me know

      strcpy(strConversionBuffer, json["startTime"]);
      uint16_t calcedStartTime =
          timeStrToMinsSinceMidnight(strConversionBuffer);

      strcpy(strConversionBuffer, json["stopTime"]);
      uint16_t calcedStopTime = timeStrToMinsSinceMidnight(strConversionBuffer);

      setValveTime(valveID, timeInDay, calcedStartTime, calcedStopTime);
      Serial.println("setValveTime");
      putValvesTimesInEEPROM();
      Serial.println("putValvesTimesInEEPROM");
      getValvesTimesFromEEPROM();
      Serial.println("getValvesTimesFromEEPROM");
      sendValveTimesToClients(valveID);
      Serial.println("sendValvesTimesToClients");

      return;
    }

    if (strcmp(action, "commitValveTimes") == 0) {
      EEPROM.commit();
      return;
    }
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

void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("Cannot mount SPIFFS volume...");
    while (true)
      ;
  }
}

void initValvePins() {
  for (uint8_t i = 0; i < VALVES_MAX_NUM; i++) {
    pinMode(valvePins[i], OUTPUT);
  }
}

void initWebServer() {
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.begin();
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void initWiFi() {
  if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
    Serial.println("Configuration failed.");
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Trying to connect [%s] ", WiFi.macAddress().c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.printf("\nconnected, ip: %s\n", WiFi.localIP().toString().c_str());
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

void sendValveTimesToClients(uint8_t valveID) {
  StaticJsonDocument<256> doc;
  doc["action"] = "cfgValve";
  doc["valveID"] = valveID;

  // stopTimes.add("23:59");

  JsonArray startTimes = doc.createNestedArray("startTimes");
  JsonArray stopTimes = doc.createNestedArray("stopTimes");

  for (uint8_t i = 0; i < VALVES_MAX_RUNS_IN_DAY; i++) {
    startTimes.add(minsSinceMidnightToTimestr(valveTimes.start[valveID][i]));
    stopTimes.add(minsSinceMidnightToTimestr(valveTimes.stop[valveID][i]));
  }

  char buffer[160];
  size_t len = serializeJson(doc, buffer);
  ws.textAll(buffer, len);
}

/*void sendRTCTimeToClients(char timestr[9]) {
  StaticJsonDocument<64> doc;

  doc["action"] = "rtcTime";
  doc["timeStr"] = timestr;

  char jsonBuffer[41];
  serializeJson(doc, jsonBuffer);

  ws.textAll(jsonBuffer);
}
*/

void setValveTime(uint8_t valveID, uint8_t timeInDay, uint16_t calcedStartTime,
                  uint16_t calcedStopTime) {

  valveTimes.start[valveID][timeInDay] = calcedStartTime;
  valveTimes.stop[valveID][timeInDay] = calcedStopTime;
}

void putValvesTimesInEEPROM() { EEPROM.put(0, valveTimes); }

void getValvesTimesFromEEPROM() { EEPROM.get(0, valveTimes); }

void valvesLoop(uint16_t minsSinceMidnight, bool shouldLog) {

  for (uint8_t thisValve = 0; thisValve < 4; thisValve++) {

    bool shouldRun = false;

    for (uint8_t thisTime = 0; thisTime < 4; thisTime++) {

      if (shouldRun) {
        if (shouldLog) {
          Serial.println(F("valve already on"));
        }
        break;
      }

      uint16_t thisStartTime = valveTimes.start[thisValve][thisTime];
      uint16_t thisStopTime = valveTimes.stop[thisValve][thisTime];

      if ((thisStartTime == 0) && (thisStopTime == 0)) {
        shouldRun = false;
      } else if (between(minsSinceMidnight, thisStartTime, thisStopTime)) {
        shouldRun = true;
      } else {
        shouldRun = false;
      }

      if (shouldLog) {
        Serial.print("on valve #");
        Serial.print(thisValve);
        Serial.print("  time of day #");
        Serial.print(thisTime);
        Serial.print("   start time: ");
        Serial.print(thisStartTime);
        Serial.print("   stop time: ");
        Serial.print(thisStopTime);
        Serial.print("   mins since midnite: ");
        Serial.print(minsSinceMidnight);
        Serial.print("   should run: ");
        Serial.println(shouldRun);
      }
      valvesOutputValues[thisValve] = shouldRun;
    }
  }
}
