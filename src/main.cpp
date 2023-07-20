#include <Arduino.h>
#include <FastLED.h>
#include <Visualization.h>
#include <Sparkle.h>
#include <Streak.h>
#include <WIFI.h>
#include <painlessMesh.h>
#include <cmath>

/*
.                   ┌───────────────┐
.                   │0           VIN│
.                   │1           GND│
.                   │2           3.3│
.                   │3            10│
.                   │4             9│ - LED
.                   │5             8│
.                   │6             7│
.                   └───────────────┘
*/


////////////////////////////////////////////////////////////////////////////////
// WIFI
////////////////////////////////////////////////////////////////////////////////
#define   MESH_PREFIX     "wizardMesh"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

uint32_t newConnection = 0;
uint32_t lightning = 0;
void newConnectionCallback(uint32_t nodeId);
void lightningAnimation();
bool connected = false;


////////////////////////////////////////////////////////////////////////////////
// LEDS
////////////////////////////////////////////////////////////////////////////////
#define NUM_LEDS 56
#define ROWS NUM_LEDS
#define COLUMNS 1

#define NUM_LEDS_RIM 36
#define DISPLAY_LED_PIN_RIM 1

#define NUM_LEDS_BOLT 20
#define DISPLAY_LED_PIN_BOLT 1


uint_fast8_t saturation = 244;

CRGB leds[NUM_LEDS];

// Visualization * bolt;
Visualization * all;
Sparkle * sparkle;
Streak * streak;

uint_fast8_t offset = 0;

////////////////////////////////////////////////////////////////////////////////
// DATA
////////////////////////////////////////////////////////////////////////////////
uint_fast8_t lightningData [250] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 135, 135, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 77, 77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 174, 174, 182, 182, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


////////////////////////////////////////////////////////////////////////////////
// SETUP
////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  while(!Serial && millis() < 5000);
  delay(1000);
  Serial.println("setup");

  // WIFI SETUP
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onNewConnection(&newConnectionCallback);

  // LED SETUP
  Serial.println("add LEDs");
  FastLED.addLeds<WS2812B, DISPLAY_LED_PIN_RIM, RGB>(leds+20, NUM_LEDS-20).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2812B, 6, GRB>(leds, 20).setCorrection( TypicalLEDStrip );

  // bolt = new Visualization(COLUMNS, ROWS, 0, saturation, leds);
  all = new Visualization(COLUMNS, ROWS, 0, saturation, leds);
  all->setValue(128);
  sparkle = new Sparkle(NUM_LEDS, 0, 0, leds, 557);

  streak = new Streak(COLUMNS, ROWS, 0, saturation, leds);
  streak->setValue(224);
  streak->setIntervalMinMax(16, 40);
  streak->setLengthMinMax(4, 50);
  streak->inititalize(millis());
  streak->setRandomHue(true);

  FastLED.clear(true);
  FastLED.showColor(0x000F00);
  Serial.println("green");
  delay(500);
}


uint_fast32_t loggingTimestamp = 0;

////////////////////////////////////////////////////////////////////////////////
// LOOP
////////////////////////////////////////////////////////////////////////////////
void loop() {
  mesh.update();
  FastLED.clear(true);

  unsigned long currentTime = mesh.getNodeTime()/1000;
  unsigned long localTime = millis();

  if (localTime > loggingTimestamp + 2000) {
    loggingTimestamp = localTime;
    // Serial.printf("%d\t%d: FPS: %d\n",
    //   localTime,
    //   mesh.getNodeTime()/1000,
    //   FastLED.getFPS());
    // Serial.print(mesh.getNodeList().size());
    // Serial.print("\t");

    Serial.println(sin(currentTime/3000.0));

    if (mesh.getNodeList().size() == 0) {
      connected = false;
    } else {
      connected = true;
    }

    Serial.println(mesh.getNodeList().size());
  }

  // if (newConnection) {
  //   Serial.printf("New Connection, nodeId = %u\n", newConnection);
  //   newConnection = 0;
  //   lightning = 1;
  // }

  // if (lightning) {
  //   lightningAnimation();
  // } else {
  // }

  // all->setValue(sin(currentTime/400.0)*32 + 64);
  // streak->setValue(sin(currentTime/400.0)*64 + 128);

  all->cycleLoop(currentTime + offset);
  sparkle->cycleLoop(currentTime + offset);
  streak->cycleLoop(currentTime + offset);

  all->setAll();
  streak->display(currentTime + offset);
  sparkle->display(currentTime + offset);

  if (!connected) {
    leds[20] = 0x2F0000;
  } else {
    // offset+=4;  }
  }

  FastLED.show();
}


////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
void newConnectionCallback(uint32_t nodeId) {
  newConnection = nodeId;
}

void lightningAnimation() {
  all->setAllCRGB(CHSV(0, 0, lightningData[lightning]));

  lightning++;

  if (lightning >= sizeof(lightningData)/sizeof(lightningData[0])) {
    lightning = 0;
  }

}