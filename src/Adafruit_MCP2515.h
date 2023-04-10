// Adapted for use with Adafruit_BusIO.
// Modified from original `MCP2515Class` class code:
// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#ifndef __ADAFRUIT_MCP2515_H__
#define __ADAFRUIT_MCP2515_H__

#include <Adafruit_SPIDevice.h>
#include <CANController.h>

#define MCP2515_DEFAULT_CLOCK_FREQUENCY 16e6

class Adafruit_MCP2515 : public CANControllerClass {

public:
  Adafruit_MCP2515(int8_t cspin, SPIClass *theSPI = &SPI);
  Adafruit_MCP2515(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin);
  virtual ~Adafruit_MCP2515();

  virtual int begin(long baudRate);
  virtual void end();

  virtual int endPacket();

  virtual int parsePacket();

  virtual void onReceive(int intPin, void (*callback)(int));

  using CANControllerClass::filter;
  virtual int filter(int id, int mask);
  using CANControllerClass::filterExtended;
  virtual int filterExtended(long id, long mask);

  virtual int observe();
  virtual int loopback();
  virtual int sleep();
  virtual int wakeup();

  void setClockFrequency(long clockFrequency);

  void dumpRegisters(Stream &out);

private:
  void reset();

  void handleInterrupt();

  uint8_t readRegister(uint8_t address);
  void modifyRegister(uint8_t address, uint8_t mask, uint8_t value);
  void writeRegister(uint8_t address, uint8_t value);

  static Adafruit_MCP2515 *instance;
  static void onInterrupt();

  Adafruit_SPIDevice *spi_dev = NULL; ///< Pointer to SPI bus interface
  long _clockFrequency;
};

#endif
