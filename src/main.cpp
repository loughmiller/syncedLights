#include <VirtualWire.h>
#include <i2c_t3.h>

const uint_fast8_t receive_pin = 12;
const byte authByteStart = 117;
const byte authByteEnd = 115;

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

    Wire.beginTransmission(4); // transmit to device #4

    // skip the auth & messageID bytes
    for (uint_fast8_t i = 2; i < buflen - 1; i++) {
      Wire.write(buf[i]);
    }
    Wire.endTransmission();    // stop transmitting

    Serial.print("Got: ");

    for (uint_fast8_t i = 0; i < buflen; i++) {
      Serial.print(buf[i]);
      Serial.print(' ');
    }

    Serial.println();
  }
}
