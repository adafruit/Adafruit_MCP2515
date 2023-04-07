/*
 * Adafruit MCP2515 FeatherWing CAN Receiver Example
 */

#include <Adafruit_MCP2515.h>

//
// Comment/uncomment chip select (CS) pin for Feather used
//
// Feather M0, M4, ESP32-S2, ESP32-S3
#define CS_PIN (5)
// Feather RP2040
// #define CS_PIN (7)
// Feather ESP32 V1 and V2
// #define CS_PIN (14)
// Feather ESP8266
// #define CS_PIN (2)

// Set CAN bus baud rate
#define CAN_BAUDRATE (250000)

Adafruit_MCP2515 mcp(CS_PIN);

void setup() {
  Serial.begin(115200);
  while(!Serial) delay(10);

  Serial.println("MCP2515 Receiver test!");

  if (!mcp.begin(CAN_BAUDRATE)) {
    Serial.println("Error initializing MCP2515.");
    while(1) delay(10);
  }
  Serial.println("MCP2515 chip found");
}

void loop() {
  // try to parse packet
  int packetSize = mcp.parsePacket();

  if (packetSize) {
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
}