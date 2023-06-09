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
  Serial.printf("in Working Mode function");
  for (int i = 0; i < NUM_LEDS; i++) {
  leds[i] = CRGB::DarkBlue;  
  }
  FastLED.show();
  FastLED.show();
}

void setLEDsMeetingMode() {
  Serial.printf("in Meeting Mode function");
  for (int i = 0; i < NUM_LEDS; i++) {
  leds[i] = CRGB::LightBlue;  
  }
  FastLED.show();
  FastLED.show();
}

void adjustBrightness(int brightness) {
  FastLED.setBrightness(brightness);
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
        setLEDsMeetingMode();
      }
      if ((strcmp("meetingOver", (const char*) payload)) == 0) {
        setLEDsWorkingMode();
      }
      if ((strcmp("LDR<300", (const char*) payload)) == 0) {
        Serial.println("equalsLDR<300successful");
        adjustBrightness(50);
      }
      if ((strcmp("300<LDR<700", (const char*) payload)) == 0) {
        Serial.println("equals300<LDR<700Successful");
        adjustBrightness(150);
      }
      if ((strcmp("700<LDR", (const char*) payload)) == 0) {
        Serial.println("equals700<LDRSuccessful");
        adjustBrightness(250);
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

  setLEDsWorkingMode();

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

//test to see if braching works

void loop() {
  webSocket.loop();
}