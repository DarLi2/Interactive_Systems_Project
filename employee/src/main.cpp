#include <Arduino.h>

#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>

#include <FastLED.h>
#define LED_PIN D4
#define NUM_LEDS 60

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

CRGB leds[NUM_LEDS];

void setLEDsWorkingMode() {
  for (int i = 0; i < NUM_LEDS; i++) {
  leds[i] = CRGB::DarkBlue;  
  }
  FastLED.show();
  FastLED.show();
}

void setLEDsMeetingMode() {
  for (int i = 0; i < NUM_LEDS; i++) {
  leds[i] = CRGB::LightBlue;  
  }
  FastLED.show();
  FastLED.show();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED: {
      Serial.printf("[WSc] Disconnected!\n");
      break;
    }
    case WStype_CONNECTED: {
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      // send message to server when Connected
      webSocket.sendTXT("Connected");
    }
      break;
    case WStype_TEXT: {
      Serial.printf("[WSc] get text: %s\n", payload);
      if ((strcmp("meeting", (const char*) payload)) == 0) {
        Serial.println("equalsCheckWarmSuccessful");
        setLEDsMeetingMode();
      }
      if ((strcmp("meetingOver", (const char*) payload)) == 0) {
        Serial.println("equalsCheckSeasideSuccessful");
        setLEDsWorkingMode();
      }
      break;
    }
  }
}

void setup() {

  Serial.begin(115200);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  pinMode(LED_PIN, OUTPUT);
  
  //for(uint8_t t = 4; t > 0; t--) {
  //  Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
  //  Serial.flush();
  //  delay(1000);
  //}

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("TeamNet", "thereisnospoon");

  //WiFi.disconnect();
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // server address, port and URL
  webSocket.begin("192.168.4.1", 81, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2);
}

void loop() {
  webSocket.loop();
}