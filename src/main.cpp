#include <Arduino.h>
#include <RH_ASK.h>
#include <SPI.h>
#include <FastLED.h>
#include <Visualization.h>
#include <Sparkle.h>
#include <Spectrum2.h>

/*
.                   ┌───────────────┐
.                   │0           VIN│
.               LED-│1           GND│
.                   │2           3.3│
.                   │3            10│
.                   │4             9│
.                   │5             8│
.                   │6             7│
.                   └───────────────┘
*/

////////////////////////////////////////////////////////////////////////////////////////////////
// RECEIVER
////////////////////////////////////////////////////////////////////////////////////////////////
RH_ASK driver(2000, 7);

const uint_fast8_t receive_pin = 7;
const byte authByteStart = 117;
const byte authByteEnd = 115;

// message types
const byte typeCycle = 1;
const byte typeBrightness = 2;
const byte typeDensity = 3;
const byte typeSparkles = 4;
const byte typeHue = 5;
const byte typeStreaks = 7;
const byte typeSolid = 8;
const byte typeSteal = 9;

byte messageType = 0;
byte messageData = 0;
uint32_t sync = 0;


////////////////////////////////////////////////////////////////////////////////////////////////
// LEDS
////////////////////////////////////////////////////////////////////////////////////////////////
#define NUM_LEDS 100
#define ROWS 1
#define COLUMNS 100
#define DISPLAY_LED_PIN 1

CRGB leds[NUM_LEDS];
CRGB off;

Visualization * all;
Sparkle * sparkle;

void setAll(CRGB color);

void setup() {
  while(!Serial && millis() < 10000);
  Serial.println("setup");

  // Initialise radiohead
  if (!driver.init()) {
    Serial.println("init failed");
  }

  // LED SETUP
  FastLED.addLeds<WS2812B, DISPLAY_LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );;
  all = new Visualization(COLUMNS, ROWS, 0, 244, leds);
  all->setValue(64);
  sparkle = new Sparkle(NUM_LEDS, 0, 0, leds, 557);
}

uint_fast8_t lastMessageID = 255;
uint_fast32_t lastLog = 0;
uint_fast32_t lastShow = 0;

void loop() {
  setAll(0x000000);  // clear leds
  uint_fast32_t currentTime = millis();

  // if (currentTime > lastLog + 5000) {
  //   Serial.print(currentTime);
  //   Serial.print("\t");
  //   Serial.print(sparkle->getHue());
  //   Serial.println();
  //   lastLog = currentTime;
  // }

  uint8_t buf[12];
  uint8_t buflen = sizeof(buf);

  if (driver.recv(buf, &buflen)) {
    if ((buf[0] != authByteStart) || (buf[buflen - 1] != authByteEnd)) {
      Serial.println("bad message");
      return;
    }

    uint_fast8_t messageID = buf[1];

    if (messageID == lastMessageID) {
      return;
    }

    lastMessageID = messageID;

    // LOGGING
    // Serial.print("Got: ");

    // for (uint_fast8_t i = 0; i < buflen; i++) {
    //   Serial.print(buf[i]);
    //   Serial.print(' ');
    // }

    // Serial.println();

    // END LOGGING

    messageType = buf[2];

    if (messageType == 10 && buflen == 8) {
      // sync
      sync = buf[3] << 24 | buf[4] << 16 | buf[5] << 8 | buf[6];
      Serial.print("sync: ");
      Serial.println(sync);
      // sparkle->synchronize(currentTime, sync);
      all->synchronize(currentTime, sync);
    } else if (messageType > 0) {
      messageData = buf[3];

      Serial.print("Control message:");
      Serial.print("\t");
      Serial.print(messageType);
      Serial.print("\t");
      Serial.println(messageData);

      switch(messageType) {
        case typeSteal:
          // stealColorAnimation(messageData);
          // changeAllHues(messageData);
          Serial.println("Steal Color.");
          break;
        case typeCycle:
          all->setCycle(messageData);
          Serial.println("Cycle Colors.");
          break;
        case typeBrightness:
          // brightness = messageData;
          // setBrightness();
          Serial.print("Brightness: ");
          Serial.println(messageData);
          break;
        case typeDensity:
          // setDensity(messageData);
          Serial.print("Density: ");
          Serial.println(messageData);
          break;
        case typeSparkles:
          // setSparkles(messageData);
          Serial.print("Sparkles: ");
          Serial.println(messageData);
          break;
        case typeStreaks:
          // setStreaks(messageData);
          Serial.print("Streaks: ");
          Serial.println(messageData);
          break;
        case typeHue:
          all->setHue(messageData);
          Serial.print("Hue: ");
          Serial.println(messageData);
          break;
      }
    }
  }


  all->cycleLoop(currentTime);
  all->setAll();

  sparkle->display(currentTime);
  FastLED.show();
}

void setAll(CRGB color) {
  for (uint_fast16_t i=0; i<NUM_LEDS; i++) {
    leds[i] = color;
  }
}