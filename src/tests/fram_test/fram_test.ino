/*
  Haptic Harpsichord: LED Test

  Testing functionality of LEDs in a 49-key strip for the haptic harpsichord.

  Example code to interrogate Adafruit SPI FRAM breakout for address size and
  storage capacity 

  NOTE: This sketch will overwrite data already on the FRAM breakout!

  | Connection      | Pin   |
  | --------------- | ----- |
  | NC              | D13   |
  | EEPROM VCC      | 3.3v  |
  | AREF POT Switch | AREF  |
  | PCB 0 QRE1113   | A0    |
  | PCB 1 QRE1113   | A1    |
  | PCB 2 QRE1113   | A2    |
  | PCB 3 QRE1113   | A3    |
  | PCB 4 QRE1113   | A4    |
  | PCB 5 QRE1113   | A6    |
  | PCB 6 QRE1113   | A7    |
  | NC              | 5v    |
  | NC              | RESET |
  | GND             | GND   |
  | PSU `+`         | VIN   |

  | Connection      | Pin     |
  | --------------- | ------- |
  | Rotary CLK      | D12     |
  | Rotary DATA     | D11     |
  | Rotary Switch   | D10     |
  | LED Data        | D9      |
  | Mux C           | D8      |
  | Mux B           | D7      |
  | Mux A           | D6      |
  | EEPROM SPI CLK  | D5      |
  | EEPROM SPI MISO | D4      |
  | EEPROM SPI MOSI | D3      |
  | EEPROM SPI CS   | D2      |
  | NC              | RX / D1 |
  | NC              | TX / D0 |

 */
//-----------------------------------------------------------------------------
#include "Adafruit_FRAM_SPI.h"
#include <SPI.h>
//-----------------------------------------------------------------------------
const uint8_t FRAM_CS = 2;
const uint8_t FRAM_MOSI = 3;
const uint8_t FRAM_MISO = 4;
const uint8_t FRAM_SCK = 5;
Adafruit_FRAM_SPI fram = Adafruit_FRAM_SPI(FRAM_SCK, FRAM_MISO, FRAM_MOSI, FRAM_CS);
//-----------------------------------------------------------------------------
uint8_t addrSizeInBytes = 2;  // Default to address size of two bytes
uint32_t memSize;
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
bool testAddrSize(uint8_t addrSize) {
  fram.setAddressSize(addrSize);
  if (readBack(4, 0xbeefbead) == 0xbeefbead)
    return true;
  return false;
}
//-----------------------------------------------------------------------------
void setup(void) {
  Serial.begin(9600);

  while (!Serial)
    delay(10);  // will pause Zero, Leonardo, etc until serial console opens

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
//-----------------------------------------------------------------------------
void loop(void) {}
//-----------------------------------------------------------------------------