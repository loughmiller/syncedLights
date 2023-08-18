#include <Arduino.h>
#include <FastLED.h>
#include <Visualization.h>
#include <Sparkle.h>
#include <Streak.h>
#include <WIFI.h>
#include <painlessMesh.h>
#include <cmath>
// #include <UMS3.h>

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
#define NUM_LEDS 50
#define ROWS 50
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
  // LED SETUP
  FastLED.addLeds<WS2812B, DISPLAY_LED_PIN, RGB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.clear(true);
  FastLED.showColor(0x000900);

  Serial.begin(9600);
  // while(!Serial && millis() < 5000);
  delay(1000);
  Serial.println("setup");

  Serial.println(setCpuFrequencyMhz(80));
  Serial.println(getCpuFrequencyMhz());

  // ums3.begin();

  // WIFI SETUP
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onNewConnection(&newConnectionCallback);

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

  // Serial.println("setup complete");
  // Serial.println("setup complete");
  // Serial.println(ARDUINO_TINYS3);
}

uint_fast32_t loggingTimestamp = 0;

////////////////////////////////////////////////////////////////////////////////
// LOOP
////////////////////////////////////////////////////////////////////////////////
void loop() {
  // Serial.println("loop");
  mesh.update();
  // Serial.println("mesh updated");
  FastLED.clear(true);
  // Serial.println("LEDs Cleared");

  if (digitalRead(33)) {
    Serial.println("charging");
    FastLED.clear();
    leds[0] = 0x002F00;
    FastLED.show();
    delay(5000);

    return;
  }

  unsigned long currentTime = mesh.getNodeTime()/1000;
  unsigned long localTime = millis();

  if (localTime > loggingTimestamp + 2000) {
    Serial.println("logging");
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

    // mesh.sendBroadcast((String)ums3.getBatteryVoltage());
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
