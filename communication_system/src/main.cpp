#include <Arduino.h>
#include <LittleFS.h>
#include <WebSockets.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#ifndef APSSID
#define APSSID "TeamNet"
#define APPSK  "thereisnospoon"
#endif

#include <FastLED.h>
#define LED_PIN D8
#define LDR_PIN A0
#define NUM_LEDS 60

SoftwareSerial mySoftwareSerial(D4,D3); //first argument: number of pin for transmitting; second argument: numver of pin for receiving
DFRobotDFPlayerMini DFPlayer;

CRGB leds[NUM_LEDS];

const char *ssid = APSSID;
const char *password = APPSK;
int LDRValue = 0;



ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void handleRoot() {
  File file = LittleFS.open("/webpage.html", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void updateStatus(char status, int employee);
void meeting();
void meetingOver();

void webSocketEvent(unsigned char num, WStype_t type, uint8_t* payload, unsigned int lenght) {
  if (type == WStype_CONNECTED) {
    Serial.println("WebSocketClient connected");
    webSocket.broadcastTXT("newDeviceConnected", 19);
  }
  if (type == WStype_DISCONNECTED) {
    Serial.println("WebSocketClient disconnected");
  }
  if (type == WStype_TEXT) {
    if ((strcmp("workingOnTaskEmployee1", (const char*) payload)) == 0) {
      updateStatus('w', 1);
      webSocket.broadcastTXT("statusUpdate_workingOnTaskEmployee1", 36);
    }
    if ((strcmp("workingOnTaskEmployee2", (const char*) payload)) == 0) {
      updateStatus('w', 2);
      webSocket.broadcastTXT("statusUpdate_workingOnTaskEmployee2", 36);
    }
    if ((strcmp("finishedTaskEmployee1", (const char*) payload)) == 0) {
      updateStatus('f', 1);
      webSocket.broadcastTXT("statusUpdate_finishedTaskEmployee1", 35);
    }
    if ((strcmp("finishedTaskEmployee2", (const char*) payload)) == 0) {
      updateStatus('f', 2);
      webSocket.broadcastTXT("statusUpdate_finishedTaskEmployee2", 35);
    }
    if ((strcmp("questionEmployee1", (const char*) payload)) == 0) {
      updateStatus('q', 1);
      webSocket.broadcastTXT("statusUpdate_questionEmployee1", 31);
    }
    if ((strcmp("questionEmployee2", (const char*) payload)) == 0) {
      updateStatus('q', 2);
      webSocket.broadcastTXT("statusUpdate_questionEmployee2", 31);
    }
    if ((strcmp("urgentQuestionEmployee1", (const char*) payload)) == 0) {
      updateStatus('u', 1);
      DFPlayer.play(1);
      webSocket.broadcastTXT("statusUpdate_urgentQuestionEmployee1", 37);
    }
    if ((strcmp("urgentQuestionEmployee2", (const char*) payload)) == 0) {
      updateStatus('u', 2);
      DFPlayer.play(2);
      webSocket.broadcastTXT("statusUpdate_urgentQuestionEmployee2", 37);
    }
    if ((strcmp("meeting", (const char*) payload)) == 0) {
      meeting();
      webSocket.broadcastTXT("meeting", 8);
    }
    if ((strcmp("meetingOver", (const char*) payload)) == 0) {
      meetingOver();
      webSocket.broadcastTXT("meetingOver", 12);
    }
  }
}

char savedStatusEmployee1;
char savedStatusEmployee2;

void setup() {

  Serial.begin(115200);
  Serial.println();

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  pinMode(D8, OUTPUT);


  mySoftwareSerial.begin(9600);
  if (!DFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  DFPlayer.volume(20);

  Serial.print("Configuring access point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  LittleFS.begin();
  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

}

void setLEDs(int start, int end, char status) {
  CRGB color = CRGB(0,0,0);
  switch(status) {
	  case 'w': color = CRGB(0, 0, 255); break;
	  case 'f': color = CRGB(0, 255, 0); break;
	  case 'q': color = CRGB(255, 255, 0); break;
    case 'u': color = CRGB(255, 0, 0); 
  }
  for (int i=start; i<end; i++) {
    leds[i] = color;
  }
  FastLED.show();
  FastLED.show();
}

void meeting() {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = CRGB::LightBlue;
  }
  FastLED.show();
  FastLED.show();
}

void meetingOver() {
  setLEDs(0, 30, savedStatusEmployee1);
  setLEDs(30, NUM_LEDS, savedStatusEmployee2);
}

void updateStatus(char status, int employee) {
  int start;
  int end;
  if (employee == 1) {
    savedStatusEmployee1 = status;
    start = 0;
    end = 30;
  }
  if (employee == 2) {
    savedStatusEmployee2 = status;
    start = 30;
    end = NUM_LEDS;
  }
  setLEDs(start, end, status);
}

unsigned int previousTime = 0;
const unsigned long interval = 1000; 
int brightness = 255;
char previousLDRValue = 'l';

void loop() {

  server.handleClient();
  webSocket.loop();

  unsigned long currentTime = millis();  

  if (currentTime - previousTime >= interval) {
    LDRValue = analogRead(LDR_PIN);
    Serial.print("LDRValue:");
    Serial.println(LDRValue);
    if (LDRValue < 300) {
      brightness = 30;
      if (previousLDRValue != 'l') {
      webSocket.broadcastTXT("LDR<300", 8);
      }
      previousLDRValue = 'l';
    }
    else if (LDRValue < 700) {
      brightness = 150;
      if (previousLDRValue != 'm') {
      webSocket.broadcastTXT("300<LDR<700", 12);
      }
      previousLDRValue = 'm';
    }
    else {
      brightness = 250;
      if (previousLDRValue != 'h') {
      webSocket.broadcastTXT("700<LDR", 8);
      }
      previousLDRValue = 'h';
    }
    Serial.println(brightness);
    FastLED.setBrightness(brightness);
    FastLED.show();
    FastLED.show();
    previousTime = currentTime;
  }

}




