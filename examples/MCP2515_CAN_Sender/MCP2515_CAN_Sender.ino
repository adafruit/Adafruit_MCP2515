/*
 * Adafruit MCP2515 FeatherWing CAN Sender Example
 */

#include <Adafruit_MCP2515.h>

// Set pin attached to CS
#define CS_PIN 5

Adafruit_MCP2515 mcp(CS_PIN);

void setup() {
  Serial.begin(9600);
  while(!Serial);

  if (!mcp.begin()) {
    Serial.println("Error initializing MCP2515.");
    while(1);
  }
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