/*
 * Report and Edit the values of the FRAM
 *
 * This sketch is used for debugging the FRAM and threshold values.
 *
 * Wiring Table:
 *
 * |     Connection | Pin Left | Pin Right | Connection      |
 * | -------------: | :------- | --------: | :-------------- |
 * | EEPROM SPI CLK | D13      |       D12 | Rotary CLK      |
 * |     EEPROM VCC | 3.3v     |       D11 | Rotary DATA     |
 * |             NC | AREF     |       D10 | Rotary Switch   |
 * |          PCB 0 | A0       |        D9 | LED Data        |
 * |          PCB 1 | A1       |        D8 | Mux C           |
 * |          PCB 2 | A2       |        D7 | Mux B           |
 * |          PCB 3 | A3       |        D6 | Mux A           |
 * |          PCB 4 | A4       |        D5 | Mux Interrupt   |
 * |          PCB 5 | A5       |        D4 | EEPROM SPI MISO |
 * |          PCB 6 | A6       |        D3 | EEPROM SPI MOSI |
 * |             NC | A7       |        D2 | EEPROM SPI CS   |
 * |             NC | 5v       |       GND | GND             |
 * |             NC | RESET    |       RST | Reset Switch    |
 * |            GND | GND      |   RX / D1 | NC              |
 * |        PSU `+` | VIN      |   TX / D0 | NC              |
 *
 */
#include <Adafruit_FRAM_SPI.h>
// FRAM Pins
const uint8_t FRAM_CS = 2;
const uint8_t FRAM_MOSI = 3;
const uint8_t FRAM_MISO = 4;
const uint8_t FRAM_SCK = 5;
Adafruit_FRAM_SPI fram =
  Adafruit_FRAM_SPI(FRAM_SCK, FRAM_MISO, FRAM_MOSI, FRAM_CS);
const uint8_t addrSizeInBytes = 2;  // Default to address size of two bytes
const uint16_t tagAddress = 0;
const uint8_t thresholdTag[4] = { 'D', 'A', 'T', 'A' };
// const uint8_t maxTag[4] = { 'M', 'A', 'X', 'I' };
// const uint8_t minTag[4] = { 'M', 'I', 'N', 'I' };
const uint16_t pluckValAddress = tagAddress + 4;
const uint16_t numSensors = 49;
uint16_t pluckThresholds[numSensors];
uint16_t defaultThreshold = 1000;
//-----------------------------------------------------------------------------
void setup() {

  if (!fram.begin(addrSizeInBytes)) {
    Serial.println("No SPI FRAM found ... check your connections\r\n");
    halt();
  }

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  readThresholdValues();

  for (int i = 0; i < 7; i++) {
    Serial.print("------ Board ");
    Serial.print(i);
      Serial.println(" ------");
    for (int j = 0; j < 7; j++) {
      Serial.print(j);
      Serial.print(": ");
      Serial.print(pluckThresholds[j]);
      Serial.print(",");
    }
    Serial.println(",");
  }

  Serial.println();
  Serial.println();

  Serial.println("Input an option or power off the Arduino\r\n");
  inputDefaultThreshold();
}
//-----------------------------------------------------------------------------
void loop() {}

void halt() {
  while (true) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
    delay(2000);
  }
}

//-----------------------------------------------------------------------------
void printInputOptions() {
  Serial.println("Enter on of the following options");
  Serial.println("- f:  set default threshold of 1000");
  Serial.println("- e:  set custom threshold (0-4096)");
}

//-----------------------------------------------------------------------------

void inputDefaultThreshold() {
  printInputOptions();

  bool waitingForInput = true;

  while (waitingForInput) {
    if (Serial.available()) {

      char option = Serial.read();
      clearSerialInputBuffer();

      switch (option) {
        case 'f':
        case 'F':
          defaultThreshold = 1000;
          waitingForInput = false;
          break;
        case 'e':
        case 'E':
          Serial.println("Enter a value (0-4095) to initialise memory:");
          while (waitingForInput) {
            if (Serial.available()) {
              defaultThreshold = Serial.parseInt(SKIP_WHITESPACE);
              clearSerialInputBuffer();
              if (defaultThreshold < 4096 && defaultThreshold >= 0) {
                Serial.print("Value Entered: ");
                Serial.println(defaultThreshold);
                Serial.println("Enter a value (0-4095)");
              } else {
                waitingForInput = false;
              }
            }
          }
          break;
        default:
          Serial.print("Option ");
          Serial.print(option);
          Serial.println(" is invalid");
          break;
      }
    }
  }
}
//-----------------------------------------------------------------------------
void readThresholdValues() {
  uint32_t tagRead;

  fram.read(tagAddress, (uint8_t *)&tagRead, 4);

  if (tagRead != *((uint32_t *)thresholdTag)) {
    Serial.println("Memory Uninitialised\r\n");
    inputDefaultThreshold();
  } else {
    fram.read(pluckValAddress, (uint8_t *)pluckThresholds, numSensors * 2);
  }
}

void writeThresholdsToEEPROM() {

  fram.writeEnable(true);

  if (!fram.write(tagAddress, thresholdTag, 4))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(true);

  if (!fram.write(pluckValAddress, (uint8_t *)pluckThresholds, numSensors * 2))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(false);
}

//-----------------------------------------------------------------------------

void clearSerialInputBuffer() {
  while (Serial.available())
    Serial.read();
}