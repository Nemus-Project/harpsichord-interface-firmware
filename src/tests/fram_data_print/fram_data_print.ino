/*
  Haptic Harpsichord: LED Test

  Dump Calibration Values

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
const uint16_t numSensors = 49;
//-----------------------------------------------------------------------------
const uint8_t FRAM_CS = 2;
const uint8_t FRAM_MOSI = 3;
const uint8_t FRAM_MISO = 4;
const uint8_t FRAM_SCK = 5;
Adafruit_FRAM_SPI fram = Adafruit_FRAM_SPI(FRAM_SCK, FRAM_MISO, FRAM_MOSI, FRAM_CS);
//-----------------------------------------------------------------------------
uint8_t addrSizeInBytes = 2;  // Default to address size of two bytes
uint32_t memSize;
const uint16_t tagAddress = 0;
const uint8_t thresholdTag[4] = { 'D', 'A', 'T', 'A' };
const uint16_t pluckValAddress = tagAddress + 4;
//-----------------------------------------------------------------------------
uint16_t pluckThresholds[numSensors];
//-----------------------------------------------------------------------------
bool shouldOverwrite = false;
uint16_t defaultThreshold = 420;
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

  readPluckFromEEPROM();
}
//-----------------------------------------------------------------------------
void loop(void) {}
//-----------------------------------------------------------------------------

void readPluckFromEEPROM() {

  uint32_t tagRead;

  fram.read(tagAddress, (uint8_t*)&tagRead, 4);


  Serial.println(*((uint32_t*)thresholdTag));
  Serial.println(tagRead);

  if (tagRead != *((uint32_t*)thresholdTag) || shouldOverwrite) {

    for (int i = 0; i < numSensors; i++)
      pluckThresholds[i] = defaultThreshold;

    fram.writeEnable(true);
    Serial.println("Overwriting DATA Tag");    
    fram.write(tagAddress, thresholdTag, 4);
    fram.write(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * 4);
    fram.writeEnable(false);
  }

  fram.read(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * 2);


  Serial.println("i\t=>\tVal");

  for (int i = 0; i < numSensors; i++) {
    Serial.print(i);
    Serial.print("\t=>\t");
    Serial.println(pluckThresholds[i]);
  }
}


void writePluckToEEPROM() {
  fram.writeEnable(true);

  if (!fram.write(tagAddress, thresholdTag, 4))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(true);
  if (!fram.write(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * 2))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(false);
}

// void overwriteFram() {
//   fram.writeEnable(true);

//   if (!fram.write(tagAddress, thresholdTag, 4))
//     digitalWrite(LEDR, LOW);

//   fram.writeEnable(true);
//   if (!fram.write(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * 2))
//     digitalWrite(LEDR, LOW);

//   fram.writeEnable(false);
// }
