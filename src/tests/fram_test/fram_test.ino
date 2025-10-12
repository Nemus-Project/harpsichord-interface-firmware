/**
 * @file fram_test.ino
 * @brief FRAM diagnostic
 *
 * |     Connection | Pin Left | Pin Right | Connection      |
 * | -------------: | -------: | :-------- | :-------------- |
 * | EEPROM SPI CLK |      D13 | D12       | Rotary CLK      |
 * |     EEPROM VCC |     3.3v | D11       | Rotary DATA     |
 * |             NC |     AREF | D10       | Rotary Switch   |
 * |          PCB 0 |       A0 | D9        | LED Data        |
 * |          PCB 1 |       A1 | D8        | Mux C           |
 * |          PCB 2 |       A2 | D7        | Mux B           |
 * |          PCB 3 |       A3 | D6        | Mux A           |
 * |          PCB 4 |       A4 | D5        | Mux Interrupt   |
 * |          PCB 5 |       A5 | D4        | EEPROM SPI MISO |
 * |          PCB 6 |       A6 | D3        | EEPROM SPI MOSI |
 * |             NC |       A7 | D2        | EEPROM SPI CS   |
 * |             NC |       5v | GND       | GND             |
 * |             NC |    RESET | RST       | Reset Switch    |
 * |            GND |      GND | RX / D1   | NC              |
 * |        PSU `+` |      VIN | TX / D0   | NC              |
 * 
 * 
 */


#include "Adafruit_FRAM_SPI.h"
#include <SPI.h>

/* Example code to interrogate Adafruit SPI FRAM breakout for address size and
 * storage capacity */

/* NOTE: This sketch will overwrite data already on the FRAM breakout */
const uint8_t FRAM_CS = 2;
const uint8_t FRAM_MOSI = 3;
const uint8_t FRAM_MISO = 4;
const uint8_t FRAM_SCK = 5;
Adafruit_FRAM_SPI fram = Adafruit_FRAM_SPI(FRAM_SCK, FRAM_MISO, FRAM_MOSI, FRAM_CS);

uint8_t addrSizeInBytes = 2; // Default to address size of two bytes
uint32_t memSize;

int32_t readBack(uint32_t addr, int32_t data) {
  int32_t check = !data;
  int32_t wrapCheck, backup;
  fram.read(addr, (uint8_t *)&backup, sizeof(int32_t));
  fram.writeEnable(true);
  fram.write(addr, (uint8_t *)&data, sizeof(int32_t));
  fram.writeEnable(false);
  fram.read(addr, (uint8_t *)&check, sizeof(int32_t));
  fram.read(0, (uint8_t *)&wrapCheck, sizeof(int32_t));
  fram.writeEnable(true);
  fram.write(addr, (uint8_t *)&backup, sizeof(int32_t));
  fram.writeEnable(false);
  // Check for warparound, address 0 will work anyway
  if (wrapCheck == check)
    check = 0;
  return check;
}

bool testAddrSize(uint8_t addrSize) {
  fram.setAddressSize(addrSize);
  if (readBack(4, 0xbeefbead) == 0xbeefbead)
    return true;
  return false;
}

void setup(void) {
  Serial.begin(9600);

  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  if (fram.begin(addrSizeInBytes)) {
    Serial.println("Found SPI FRAM");
  } else {
    Serial.println("No SPI FRAM found ... check your connections\r\n");
    while (1)
      ;
  }

  if (testAddrSize(2))
    addrSizeInBytes = 2;
  else if (testAddrSize(3))
    addrSizeInBytes = 3;
  else if (testAddrSize(4))
    addrSizeInBytes = 4;
  else {
    Serial.println(
        "SPI FRAM can not be read/written with any address size\r\n");
    while (1)
      ;
  }

  memSize = 0;
  while (readBack(memSize, memSize) == memSize) {
    memSize += 256;
    // Serial.print("Block: #"); Serial.println(memSize/256);
  }

  Serial.print("SPI FRAM address size is ");
  Serial.print(addrSizeInBytes);
  Serial.println(" bytes.");
  Serial.println("SPI FRAM capacity appears to be..");
  Serial.print(memSize);
  Serial.println(" bytes");
  Serial.print(memSize / 0x400);
  Serial.println(" kilobytes");
  Serial.print((memSize * 8) / 0x400);
  Serial.println(" kilobits");
  if (memSize >= (0x100000 / 8)) {
    Serial.print((memSize * 8) / 0x100000);
    Serial.println(" megabits");
  }
}

void loop(void) {}
