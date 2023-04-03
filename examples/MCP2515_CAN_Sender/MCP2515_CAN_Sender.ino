/*
 * Adafruit MCP2515 FeatherWing CAN Sender Example
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
#define CAN_BAUDRATE (500000)

Adafruit_MCP2515 mcp(CS_PIN);

void setup() {
  Serial.begin(115200);
  while(!Serial) delay(10);

  Serial.println("MCP2515 Sender test!");

  if (!mcp.begin(CAN_BAUDRATE)) {
    Serial.println("Error initializing MCP2515.");
    while(1) delay(10);
  }
  Serial.println("MCP2515 chip found");
}

void loop() {
  // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  Serial.print("Sending packet ... ");

  mcp.beginPacket(0x12);
  mcp.write('h');
  mcp.write('e');
  mcp.write('l');
  mcp.write('l');
  mcp.write('o');
  mcp.endPacket();

  Serial.println("done");

  delay(1000);

  // send extended packet: id is 29 bits, packet can contain up to 8 bytes of data
  Serial.print("Sending extended packet ... ");

  mcp.beginExtendedPacket(0xabcdef);
  mcp.write('w');
  mcp.write('o');
  mcp.write('r');
  mcp.write('l');
  mcp.write('d');
  mcp.endPacket();

  Serial.println("done");

  delay(1000);
}