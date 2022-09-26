#include <Arduino.h>
#include <RH_ASK.h>
#include <SPI.h>
#include <FastLED.h>
#include <Visualization.h>
#include <Sparkle.h>
#include <Streak.h>

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

const uint_fast8_t maxBrightness = 64;

// FUNCTIONS
void stealColorAnimation(uint_fast8_t hue);

////////////////////////////////////////////////////////////////////////////////////////////////
// LEDS
////////////////////////////////////////////////////////////////////////////////////////////////
#define NUM_LEDS 100
#define ROWS 100
#define COLUMNS 1
#define DISPLAY_LED_PIN 1

uint_fast8_t saturation = 244;

CRGB leds[NUM_LEDS];
CRGB off;

Visualization * all;
Sparkle * sparkle;
Streak * streak;

void setup() {
  while(!Serial && millis() < 10000);
  Serial.println("setup");

  // Initialise radiohead
  if (!driver.init()) {
    Serial.println("init failed");
  }

  // LED SETUP
  FastLED.addLeds<WS2812B, DISPLAY_LED_PIN, RGB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );;
  all = new Visualization(COLUMNS, ROWS, 0, saturation, leds);
  all->setValue(32);
  sparkle = new Sparkle(NUM_LEDS, 0, 0, leds, 557);

  streak = new Streak(COLUMNS, ROWS, 0, saturation, leds);
  streak->setValue(128);
  streak->setIntervalMinMax(16, 40);
  streak->setLengthMinMax(4, 50);
  streak->inititalize(millis());
  streak->setRandomHue(true);

  all->setAllCRGB(0x000F00);
  FastLED.show();
  delay(1500);
}

uint_fast8_t lastMessageID = 255;
uint_fast32_t lastLog = 0;
uint_fast32_t lastShow = 0;

void loop() {
  all->clearAll();  // clear leds
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
      sparkle->synchronize(currentTime, sync);
      streak->synchronize(currentTime, sync);
      all->synchronize(currentTime, sync);
    } else if (messageType > 0) {
      messageData = buf[3];

      Serial.print("Control message:");
      Serial.print("\t");
      Serial.print(messageType);
      Serial.print("\t");
      Serial.println(messageData);

     float percentBrightness = 0;


      switch(messageType) {
        case typeSteal:
          stealColorAnimation(messageData);
          Serial.println("Steal Color.");
          break;
        case typeCycle:
          all->setCycle(messageData);
          sparkle->setCycle(messageData);
          Serial.println("Cycle Colors.");
          break;
        case typeBrightness:
          percentBrightness =  (float)messageData / 256.0;
          all->setValue(percentBrightness * maxBrightness);
          Serial.print("Brightness: ");
          Serial.println(messageData);
          break;
        case typeSparkles:
          sparkle->setEmptiness(4294967295/((float)pow(messageData, 3.1)));
          Serial.print("Sparkles: ");
          Serial.println(messageData);
          break;
        case typeStreaks:
          // setStreaks(messageData);
          Serial.print("Streaks: ");
          Serial.println(messageData);
          break;
        case typeHue:
          all->setCycle(0);
          all->setHue(messageData);
          Serial.print("Hue: ");
          Serial.println(messageData);
          break;
      }
    }
  }


  all->cycleLoop(currentTime);
  sparkle->cycleLoop(currentTime);
  streak->cycleLoop(currentTime);

  all->setAll();
  streak->display(currentTime);
  sparkle->display(currentTime);
  FastLED.show();
}

void stealColorAnimation(uint_fast8_t hue) {
  CRGB color = CHSV(hue, saturation, maxBrightness);
  all->clearAll();

  uint_fast8_t delayMS = 100;

  for (uint_fast16_t y=0; y<NUM_LEDS; y++) {
    leds[y] = color;
    FastLED.show();
    delay(delayMS);
    delayMS = max(delayMS * 0.99, 1);
  }
}