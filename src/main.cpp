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
#define NUM_LEDS 100
#define ROWS 100
#define COLUMNS 1
#define DISPLAY_LED_PIN 1

uint_fast8_t saturation = 244;

CRGB leds[NUM_LEDS];

Visualization * all;
Sparkle * sparkle;
Streak * streak;


////////////////////////////////////////////////////////////////////////////////
// SETUP
////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  while(!Serial && millis() < 5000);
  Serial.println("setup");

  // WIFI SETUP
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onNewConnection(&newConnectionCallback);

  // LED SETUP
  FastLED.addLeds<WS2812B, DISPLAY_LED_PIN, RGB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

  all = new Visualization(COLUMNS, ROWS, 0, saturation, leds);
  all->setValue(32);
  sparkle = new Sparkle(NUM_LEDS, 0, 0, leds, 557);

  streak = new Streak(COLUMNS, ROWS, 0, saturation, leds);
  streak->setValue(128);
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
    // Serial.println(mesh.isConnected(1976237668));


    if (mesh.getNodeList().size() == 0) {
      connected = false;
    } else {
      connected = true;
    }
  }

  if (newConnection) {
    Serial.printf("New Connection, nodeId = %u\n", newConnection);
    newConnection = 0;
  }

  all->cycleLoop(currentTime);
  sparkle->cycleLoop(currentTime);
  streak->cycleLoop(currentTime);

  all->setAll();
  streak->display(currentTime);
  sparkle->display(currentTime);

  if (!connected) {
    leds[0] = 0x2F0000;
  }

  FastLED.show();

}


////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
void newConnectionCallback(uint32_t nodeId) {
  newConnection = nodeId;
}
