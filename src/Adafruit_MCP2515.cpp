// Adapted for use with Adafruit_BusIO.
// Modified from original code:
// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include "Adafruit_MCP2515.h"

#define REG_BFPCTRL 0x0c
#define REG_TXRTSCTRL 0x0d

#define REG_CANCTRL 0x0f

#define REG_CNF3 0x28
#define REG_CNF2 0x29
#define REG_CNF1 0x2a

#define REG_CANINTE 0x2b
#define REG_CANINTF 0x2c

#define FLAG_RXnIE(n) (0x01 << n)
#define FLAG_RXnIF(n) (0x01 << n)
#define FLAG_TXnIF(n) (0x04 << n)

#define REG_RXFnSIDH(n) (0x00 + (n * 4))
#define REG_RXFnSIDL(n) (0x01 + (n * 4))
#define REG_RXFnEID8(n) (0x02 + (n * 4))
#define REG_RXFnEID0(n) (0x03 + (n * 4))

#define REG_RXMnSIDH(n) (0x20 + (n * 0x04))
#define REG_RXMnSIDL(n) (0x21 + (n * 0x04))
#define REG_RXMnEID8(n) (0x22 + (n * 0x04))
#define REG_RXMnEID0(n) (0x23 + (n * 0x04))

#define REG_TXBnCTRL(n) (0x30 + (n * 0x10))
#define REG_TXBnSIDH(n) (0x31 + (n * 0x10))
#define REG_TXBnSIDL(n) (0x32 + (n * 0x10))
#define REG_TXBnEID8(n) (0x33 + (n * 0x10))
#define REG_TXBnEID0(n) (0x34 + (n * 0x10))
#define REG_TXBnDLC(n) (0x35 + (n * 0x10))
#define REG_TXBnD0(n) (0x36 + (n * 0x10))

#define REG_RXBnCTRL(n) (0x60 + (n * 0x10))
#define REG_RXBnSIDH(n) (0x61 + (n * 0x10))
#define REG_RXBnSIDL(n) (0x62 + (n * 0x10))
#define REG_RXBnEID8(n) (0x63 + (n * 0x10))
#define REG_RXBnEID0(n) (0x64 + (n * 0x10))
#define REG_RXBnDLC(n) (0x65 + (n * 0x10))
#define REG_RXBnD0(n) (0x66 + (n * 0x10))

#define FLAG_IDE 0x08
#define FLAG_SRR 0x10
#define FLAG_RTR 0x40
#define FLAG_EXIDE 0x08

#define FLAG_RXM0 0x20
#define FLAG_RXM1 0x40

Adafruit_MCP2515::Adafruit_MCP2515(int8_t cspin, SPIClass *theSPI)
    : CANControllerClass(), _clockFrequency(MCP2515_DEFAULT_CLOCK_FREQUENCY) {
  spi_dev = new Adafruit_SPIDevice(cspin, 10e6, SPI_BITORDER_MSBFIRST,
                                   SPI_MODE0, theSPI);
}

Adafruit_MCP2515::Adafruit_MCP2515(int8_t cspin, int8_t mosipin, int8_t misopin,
                                   int8_t sckpin)
    : CANControllerClass(), _clockFrequency(MCP2515_DEFAULT_CLOCK_FREQUENCY) {
  spi_dev = new Adafruit_SPIDevice(cspin, sckpin, misopin, mosipin, 10e6);
}

Adafruit_MCP2515::~Adafruit_MCP2515() {}

int Adafruit_MCP2515::begin(long baudRate) {
  CANControllerClass::begin(baudRate);

  if (!spi_dev->begin())
    return 0;

  reset();

  writeRegister(REG_CANCTRL, 0x80);
  if (readRegister(REG_CANCTRL) != 0x80) {
    return 0;
  }

  const struct {
    long clockFrequency;
    long baudRate;
    uint8_t cnf[3];
  } CNF_MAPPER[] = {
      {(long)8E6, (long)1000E3, {0x00, 0x80, 0x00}},
      {(long)8E6, (long)500E3, {0x00, 0x90, 0x02}},
      {(long)8E6, (long)250E3, {0x00, 0xb1, 0x05}},
      {(long)8E6, (long)200E3, {0x00, 0xb4, 0x06}},
      {(long)8E6, (long)125E3, {0x01, 0xb1, 0x05}},
      {(long)8E6, (long)100E3, {0x01, 0xb4, 0x06}},
      {(long)8E6, (long)80E3, {0x01, 0xbf, 0x07}},
      {(long)8E6, (long)50E3, {0x03, 0xb4, 0x06}},
      {(long)8E6, (long)40E3, {0x03, 0xbf, 0x07}},
      {(long)8E6, (long)20E3, {0x07, 0xbf, 0x07}},
      {(long)8E6, (long)10E3, {0x0f, 0xbf, 0x07}},
      {(long)8E6, (long)5E3, {0x1f, 0xbf, 0x07}},

      {(long)16E6, (long)1000E3, {0x00, 0xd0, 0x82}},
      {(long)16E6, (long)500E3, {0x00, 0xf0, 0x86}},
      {(long)16E6, (long)250E3, {0x41, 0xf1, 0x85}},
      {(long)16E6, (long)200E3, {0x01, 0xfa, 0x87}},
      {(long)16E6, (long)125E3, {0x03, 0xf0, 0x86}},
      {(long)16E6, (long)100E3, {0x03, 0xfa, 0x87}},
      {(long)16E6, (long)80E3, {0x03, 0xff, 0x87}},
      {(long)16E6, (long)50E3, {0x07, 0xfa, 0x87}},
      {(long)16E6, (long)40E3, {0x07, 0xff, 0x87}},
      {(long)16E6, (long)20E3, {0x0f, 0xff, 0x87}},
      {(long)16E6, (long)10E3, {0x1f, 0xff, 0x87}},
      {(long)16E6, (long)5E3, {0x3f, 0xff, 0x87}},
  };

  const uint8_t *cnf = NULL;

  for (unsigned int i = 0; i < (sizeof(CNF_MAPPER) / sizeof(CNF_MAPPER[0]));
       i++) {
    if (CNF_MAPPER[i].clockFrequency == _clockFrequency &&
        CNF_MAPPER[i].baudRate == baudRate) {
      cnf = CNF_MAPPER[i].cnf;
      break;
    }
  }

  if (cnf == NULL) {
    return 0;
  }

  writeRegister(REG_CNF1, cnf[0]);
  writeRegister(REG_CNF2, cnf[1]);
  writeRegister(REG_CNF3, cnf[2]);

  writeRegister(REG_CANINTE, FLAG_RXnIE(1) | FLAG_RXnIE(0));
  writeRegister(REG_BFPCTRL, 0x00);
  writeRegister(REG_TXRTSCTRL, 0x00);
  writeRegister(REG_RXBnCTRL(0), FLAG_RXM1 | FLAG_RXM0);
  writeRegister(REG_RXBnCTRL(1), FLAG_RXM1 | FLAG_RXM0);

  writeRegister(REG_CANCTRL, 0x00);
  if (readRegister(REG_CANCTRL) != 0x00) {
    return 0;
  }

  instance = this;

  return 1;
}

void Adafruit_MCP2515::end() { CANControllerClass::end(); }

int Adafruit_MCP2515::endPacket() {
  if (!CANControllerClass::endPacket()) {
    return 0;
  }

  int n = 0;

  if (_txExtended) {
    writeRegister(REG_TXBnSIDH(n), _txId >> 21);
    writeRegister(REG_TXBnSIDL(n), (((_txId >> 18) & 0x07) << 5) | FLAG_EXIDE |
                                       ((_txId >> 16) & 0x03));
    writeRegister(REG_TXBnEID8(n), (_txId >> 8) & 0xff);
    writeRegister(REG_TXBnEID0(n), _txId & 0xff);
  } else {
    writeRegister(REG_TXBnSIDH(n), _txId >> 3);
    writeRegister(REG_TXBnSIDL(n), _txId << 5);
    writeRegister(REG_TXBnEID8(n), 0x00);
    writeRegister(REG_TXBnEID0(n), 0x00);
  }

  if (_txRtr) {
    writeRegister(REG_TXBnDLC(n), 0x40 | _txLength);
  } else {
    writeRegister(REG_TXBnDLC(n), _txLength);

    for (int i = 0; i < _txLength; i++) {
      writeRegister(REG_TXBnD0(n) + i, _txData[i]);
    }
  }

  writeRegister(REG_TXBnCTRL(n), 0x08);

  bool aborted = false;

  while (readRegister(REG_TXBnCTRL(n)) & 0x08) {
    if (readRegister(REG_TXBnCTRL(n)) & 0x10) {
      // abort
      aborted = true;

      modifyRegister(REG_CANCTRL, 0x10, 0x10);
    }

    yield();
  }

  if (aborted) {
    // clear abort command
    modifyRegister(REG_CANCTRL, 0x10, 0x00);
  }

  modifyRegister(REG_CANINTF, FLAG_TXnIF(n), 0x00);

  return (readRegister(REG_TXBnCTRL(n)) & 0x70) ? 0 : 1;
}

int Adafruit_MCP2515::parsePacket() {
  int n;

  uint8_t intf = readRegister(REG_CANINTF);

  if (intf & FLAG_RXnIF(0)) {
    n = 0;
  } else if (intf & FLAG_RXnIF(1)) {
    n = 1;
  } else {
    _rxId = -1;
    _rxExtended = false;
    _rxRtr = false;
    _rxLength = 0;
    return 0;
  }

  _rxExtended = (readRegister(REG_RXBnSIDL(n)) & FLAG_IDE) ? true : false;

  uint32_t idA = ((readRegister(REG_RXBnSIDH(n)) << 3) & 0x07f8) |
                 ((readRegister(REG_RXBnSIDL(n)) >> 5) & 0x07);
  if (_rxExtended) {
    uint32_t idB =
        (((uint32_t)(readRegister(REG_RXBnSIDL(n)) & 0x03) << 16) & 0x30000) |
        ((readRegister(REG_RXBnEID8(n)) << 8) & 0xff00) |
        readRegister(REG_RXBnEID0(n));

    _rxId = (idA << 18) | idB;
    _rxRtr = (readRegister(REG_RXBnDLC(n)) & FLAG_RTR) ? true : false;
  } else {
    _rxId = idA;
    _rxRtr = (readRegister(REG_RXBnSIDL(n)) & FLAG_SRR) ? true : false;
  }
  _rxDlc = readRegister(REG_RXBnDLC(n)) & 0x0f;
  _rxIndex = 0;

  if (_rxRtr) {
    _rxLength = 0;
  } else {
    _rxLength = _rxDlc;

    for (int i = 0; i < _rxLength; i++) {
      _rxData[i] = readRegister(REG_RXBnD0(n) + i);
    }
  }

  modifyRegister(REG_CANINTF, FLAG_RXnIF(n), 0x00);

  return _rxDlc;
}

void Adafruit_MCP2515::onReceive(int intPin, void (*callback)(int)) {
  CANControllerClass::onReceive(callback);

  pinMode(intPin, INPUT);

  if (callback) {
#if !defined ESP8266 && !defined ESP32
    SPI.usingInterrupt(digitalPinToInterrupt(intPin));
#endif
    attachInterrupt(digitalPinToInterrupt(intPin),
                    Adafruit_MCP2515::onInterrupt, LOW);
  } else {
    detachInterrupt(digitalPinToInterrupt(intPin));
#ifdef SPI_HAS_NOTUSINGINTERRUPT
    SPI.notUsingInterrupt(digitalPinToInterrupt(intPin));
#endif
  }
}

int Adafruit_MCP2515::filter(int id, int mask) {
  id &= 0x7ff;
  mask &= 0x7ff;

  // config mode
  writeRegister(REG_CANCTRL, 0x80);
  if (readRegister(REG_CANCTRL) != 0x80) {
    return 0;
  }

  for (int n = 0; n < 2; n++) {
    // standard only
    writeRegister(REG_RXBnCTRL(n), FLAG_RXM0);
    writeRegister(REG_RXBnCTRL(n), FLAG_RXM0);

    writeRegister(REG_RXMnSIDH(n), mask >> 3);
    writeRegister(REG_RXMnSIDL(n), mask << 5);
    writeRegister(REG_RXMnEID8(n), 0);
    writeRegister(REG_RXMnEID0(n), 0);
  }

  for (int n = 0; n < 6; n++) {
    writeRegister(REG_RXFnSIDH(n), id >> 3);
    writeRegister(REG_RXFnSIDL(n), id << 5);
    writeRegister(REG_RXFnEID8(n), 0);
    writeRegister(REG_RXFnEID0(n), 0);
  }

  // normal mode
  writeRegister(REG_CANCTRL, 0x00);
  if (readRegister(REG_CANCTRL) != 0x00) {
    return 0;
  }

  return 1;
}

int Adafruit_MCP2515::filterExtended(long id, long mask) {
  id &= 0x1FFFFFFF;
  mask &= 0x1FFFFFFF;

  // config mode
  writeRegister(REG_CANCTRL, 0x80);
  if (readRegister(REG_CANCTRL) != 0x80) {
    return 0;
  }

  for (int n = 0; n < 2; n++) {
    // extended only
    writeRegister(REG_RXBnCTRL(n), FLAG_RXM1);
    writeRegister(REG_RXBnCTRL(n), FLAG_RXM1);

    writeRegister(REG_RXMnSIDH(n), mask >> 21);
    writeRegister(REG_RXMnSIDL(n), (((mask >> 18) & 0x03) << 5) | FLAG_EXIDE |
                                       ((mask >> 16) & 0x03));
    writeRegister(REG_RXMnEID8(n), (mask >> 8) & 0xff);
    writeRegister(REG_RXMnEID0(n), mask & 0xff);
  }

  for (int n = 0; n < 6; n++) {
    writeRegister(REG_RXFnSIDH(n), id >> 21);
    writeRegister(REG_RXFnSIDL(n), (((id >> 18) & 0x03) << 5) | FLAG_EXIDE |
                                       ((id >> 16) & 0x03));
    writeRegister(REG_RXFnEID8(n), (id >> 8) & 0xff);
    writeRegister(REG_RXFnEID0(n), id & 0xff);
  }

  // normal mode
  writeRegister(REG_CANCTRL, 0x00);
  if (readRegister(REG_CANCTRL) != 0x00) {
    return 0;
  }

  return 1;
}

int Adafruit_MCP2515::observe() {
  writeRegister(REG_CANCTRL, 0x80);
  if (readRegister(REG_CANCTRL) != 0x80) {
    return 0;
  }

  return 1;
}

int Adafruit_MCP2515::loopback() {
  writeRegister(REG_CANCTRL, 0x40);
  if (readRegister(REG_CANCTRL) != 0x40) {
    return 0;
  }

  return 1;
}

int Adafruit_MCP2515::sleep() {
  writeRegister(REG_CANCTRL, 0x01);
  if (readRegister(REG_CANCTRL) != 0x01) {
    return 0;
  }

  return 1;
}

int Adafruit_MCP2515::wakeup() {
  writeRegister(REG_CANCTRL, 0x00);
  if (readRegister(REG_CANCTRL) != 0x00) {
    return 0;
  }

  return 1;
}

void Adafruit_MCP2515::setClockFrequency(long clockFrequency) {
  _clockFrequency = clockFrequency;
}

void Adafruit_MCP2515::dumpRegisters(Stream &out) {
  for (int i = 0; i < 128; i++) {
    byte b = readRegister(i);

    out.print("0x");
    if (i < 16) {
      out.print('0');
    }
    out.print(i, HEX);
    out.print(": 0x");
    if (b < 16) {
      out.print('0');
    }
    out.println(b, HEX);
  }
}

void Adafruit_MCP2515::reset() {
  uint8_t buffer[1] = {0xC0};
  spi_dev->write(buffer, 1);

  delayMicroseconds(10);
}

void Adafruit_MCP2515::handleInterrupt() {
  if (readRegister(REG_CANINTF) == 0) {
    return;
  }

  while (parsePacket()) {
    _onReceive(available());
  }
}

uint8_t Adafruit_MCP2515::readRegister(uint8_t address) {
  uint8_t buffer[2] = {0x03, address};
  spi_dev->write_then_read(buffer, 2, buffer, 1);
  return buffer[0];
}

void Adafruit_MCP2515::modifyRegister(uint8_t address, uint8_t mask,
                                      uint8_t value) {
  uint8_t buffer[4] = {0x05, address, mask, value};
  spi_dev->write(buffer, 4);
}

void Adafruit_MCP2515::writeRegister(uint8_t address, uint8_t value) {
  uint8_t buffer[3] = {0x02, address, value};
  spi_dev->write(buffer, 3);
}

void Adafruit_MCP2515::onInterrupt() { instance->handleInterrupt(); }

Adafruit_MCP2515 *Adafruit_MCP2515::instance;