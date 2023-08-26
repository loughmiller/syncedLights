#include <Arduino.h>
#include <FastLED.h>
#include <Visualization.h>
#include <Sparkle.h>
#include <Streak.h>
#include <WIFI.h>
#include <painlessMesh.h>
#include <cmath>
#include <UMS3.h>


////////////////////////////////////////////////////////////////////////////////
// Tinys3 Helpers
////////////////////////////////////////////////////////////////////////////////
UMS3 tinyS3;

////////////////////////////////////////////////////////////////////////////////
// WIFI
////////////////////////////////////////////////////////////////////////////////
#define   MESH_PREFIX     "wizardMesh"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

void newConnectionCallback(uint32_t nodeId);
void receivedCallback(uint32_t from, String &msg);
bool connected = false;
uint32_t newConnection = 0;


////////////////////////////////////////////////////////////////////////////////
// LEDS
////////////////////////////////////////////////////////////////////////////////
#define NUM_LEDS 56
#define ROWS NUM_LEDS
#define COLUMNS 1

#define NUM_LEDS_RIM 36
#define DISPLAY_LED_PIN_RIM 5

#define NUM_LEDS_BOLT 20
#define DISPLAY_LED_PIN_BOLT 34


uint_fast8_t saturation = 244;

CRGB leds[NUM_LEDS];

Visualization * bolt;
Visualization * all;
Sparkle * sparkle;
Streak * streak;


////////////////////////////////////////////////////////////////////////////////
// PROTOCOL
////////////////////////////////////////////////////////////////////////////////
// const byte typeCycle = 1;
// const byte typeBrightness = 2;
// const byte typeSparkles = 4;
// const byte typeHue = 5;
// const byte typeSteal = 9;

////////////////////////////////////////////////////////////////////////////////
// SETUP
////////////////////////////////////////////////////////////////////////////////
void setup() {
  // LED SETUP
  FastLED.addLeds<WS2812B, DISPLAY_LED_PIN_RIM, RGB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2812B, DISPLAY_LED_PIN_BOLT, GRB>(leds + NUM_LEDS_RIM, NUM_LEDS_BOLT).setCorrection( TypicalLEDStrip );
  FastLED.clear(true);
  FastLED.showColor(0x000900);

  Serial.begin(9600);
  // while(!Serial && millis() < 5000);
  delay(1000);
  Serial.println("setup");

  Serial.println(setCpuFrequencyMhz(80));
  Serial.println(getCpuFrequencyMhz());

  tinyS3.begin();

  // WIFI SETUP
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onReceive(&receivedCallback);
  Serial.println("wifi setup complete");

  all = new Visualization(COLUMNS, ROWS, 0, saturation, leds);
  all->setValue(32);
  sparkle = new Sparkle(NUM_LEDS, 0, 0, leds, 557);

  streak = new Streak(COLUMNS, ROWS, 0, saturation, leds);
  streak->setValue(128);
  streak->setIntervalMinMax(16, 40);
  streak->setLengthMinMax(4, 50);
  streak->inititalize(millis());
  streak->setRandomHue(true);

  Serial.println("setup complete");
  Serial.println("setup complete");
  Serial.println("setup complete");
  Serial.println("setup complete");
  Serial.println("setup complete");
  Serial.println("setup complete");
  Serial.println("SYNCED LIGHTS");
}

uint_fast32_t loggingTimestamp = 0;

////////////////////////////////////////////////////////////////////////////////
// LOOP
////////////////////////////////////////////////////////////////////////////////
void loop() {
  // Serial.println("loop");
  mesh.update();
  // Serial.println("mesh updated");
  FastLED.clear();
  // Serial.println("LEDs Cleared");

  unsigned long currentTime = mesh.getNodeTime()/1000;
  unsigned long localTime = millis();

  if (localTime > loggingTimestamp + 2000) {
    // Serial.println("logging");
    loggingTimestamp = localTime;
    // Serial.printf("%d\t%d: FPS: %d\n",
    //   localTime,
    //   mesh.getNodeTime()/1000,
    //   FastLED.getFPS());
    // Serial.print(mesh.getNodeList().size());
    // Serial.print("\t");
    // Serial.println(mesh.isConnected(1976237668));

    if (mesh.getNodeList().size() == 0) {
      if (connected) {
        Serial.println("disconnected");
      }
      connected = false;
    } else {
      if (!connected) {
        Serial.println("connected");
      }
      connected = true;
    }

    // if (connected) {
    //   Serial.println("Sending Voltage");
    //   mesh.sendBroadcast((String)tinyS3.getBatteryVoltage());
    // }

    // if (digitalRead(33)) {
    //   Serial.println("charging");
    // }
  }

  // // Stop Lights while charging
  // if (digitalRead(33)) {
  //   leds[0] = 0x002F00;
  //   FastLED.show();
  //   return;
  // }


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

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());

  DynamicJsonDocument config(256);
  deserializeJson(config, msg);

  if (config["sync"]) {
    uint32_t sync = config["sync"];
    // Serial.print("updating sync: ");
    // Serial.println(sync);
    all->synchronize(mesh.getNodeTime()/1000, sync);
    streak->synchronize(mesh.getNodeTime()/1000, sync);
  }

  if(config["cycle"]) {
    uint8_t cycle = config["cycle"];
    Serial.print("updating cycle: ");
    if (cycle == 255) {
      all->setCycle(0);
      streak->setCycle(0);
      streak->setRandomHue(false);
      Serial.println(0);
    } else {
      all->setCycle(cycle);
      streak->setCycle(cycle);
      streak->setRandomHue(true);
      Serial.println(cycle);
    }
  }

  if(config["brightness"]) {
    uint8_t brightness = config["brightness"];
    // Serial.print("updating brightness: ");
    // Serial.println(brightness);
    all->setValue(brightness/4);
    streak->setValue(brightness/2);
  }

  if(config["sparkles"]) {
    uint_fast32_t sparkles = config["sparkles"];
    Serial.print("updating sparkles: ");
    Serial.println(sparkles);
    //sparkle->setEmptiness(4294967295/((float)pow(sparkles, 3.1)));
    sparkle->setEmptiness(sparkles);
  }

  if(config["hue"]) {
    uint8_t hue = config["hue"];
    Serial.print("updating hue: ");
    Serial.println(hue);
    all->setHue(hue);
    streak->setHue(hue);
  }
}