/*
 * Adafruit MCP2515 FeatherWing CAN Receiver Callback Example
 */

#include <Adafruit_MCP2515.h>

// Set pin attached to CS
#define CS_PIN 5
// Set pin attached to INT
#define INT_PIN 6

Adafruit_MCP2515 mcp(CS_PIN);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("CAN Receiver Callback");

  if (!mcp.begin()) {
    Serial.println("Starting CAN failed!");
    while (1);
  }

  // register the receive callback
  mcp.onReceive(INT_PIN, onReceive);
}

void loop() {
  // do nothing
}

void onReceive(int packetSize) {
  // received a packet
  Serial.print("Received ");

  if (mcp.packetExtended()) {
    Serial.print("extended ");
  }

  if (mcp.packetRtr()) {
    // Remote transmission request, packet contains no data
    Serial.print("RTR ");
  }

  Serial.print("packet with id 0x");
  Serial.print(mcp.packetId(), HEX);

  if (mcp.packetRtr()) {
    Serial.print(" and requested length ");
    Serial.println(mcp.packetDlc());
  } else {
    Serial.print(" and length ");
    Serial.println(packetSize);

    // only print packet data for non-RTR packets
    while (mcp.available()) {
      Serial.print((char)mcp.read());
    }
    Serial.println();
  }

  Serial.println();
}


