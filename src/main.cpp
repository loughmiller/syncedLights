#include <VirtualWire.h>
#include <i2c_t3.h>

////////////////////////////////////////////////////////////////////////////////////////////////
// RECEIVER
////////////////////////////////////////////////////////////////////////////////////////////////
const uint_fast8_t receive_pin = 12;
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



void setup() {
  while(!Serial && millis() < 10000);
  Serial.println("setup");

  // Initialise the IO and ISR
  vw_set_rx_pin(receive_pin);
  vw_setup(2000);	              // Bits per sec
  vw_rx_start();                // Start the receiver PLL running

  Wire.begin(); // join i2c bus (address optional for master)
}

uint_fast8_t lastMessageID = 255;

void loop() {
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) {
    if ((buf[0] != authByteStart) || (buf[buflen - 1] != authByteEnd)) {
      // bad message
      return;
    }

    uint_fast8_t messageID = buf[1];

    if (messageID == lastMessageID) {
      return;
    }

    lastMessageID = messageID;

    // LOGGING
    Serial.print("Got: ");

    for (uint_fast8_t i = 0; i < buflen; i++) {
      Serial.print(buf[i]);
      Serial.print(' ');
    }

    Serial.println();

    // END LOGGING

    messageType = buf[2];

    if (messageType == 10 && buflen == 8) {
      // sync
      sync = buf[3] << 24 | buf[4] << 16 | buf[5] << 16 | buf[6];
      Serial.print("sync: ");
      Serial.println(sync);
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
          // setHueDrift(messageData);
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
          // changeAllHues(messageData);
          Serial.print("Hue: ");
          Serial.println(messageData);
          break;
      }
    }
  }
}