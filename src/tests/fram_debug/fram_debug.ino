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
const uint16_t numSensors = 49;
// FRAM Pins
const uint8_t FRAM_CS = 2;
const uint8_t FRAM_MOSI = 3;
const uint8_t FRAM_MISO = 4;
const uint8_t FRAM_SCK = 5;
Adafruit_FRAM_SPI fram = Adafruit_FRAM_SPI(FRAM_SCK, FRAM_MISO, FRAM_MOSI, FRAM_CS);
const uint8_t addrSizeInBytes = 2;  // Default to address size of two bytes
///
const uint16_t singleThresholdTagAddress = 0;
///
const char singleTag[4] = { 'D', 'A', 'T', 'A' };
///
const uint16_t thresholdValueAddress = singleThresholdTagAddress + 4;
///
const uint16_t pluckThresholdTagAddress = thresholdValueAddress + (numSensors * 2);
///
const char pluckTag[4] = { 'P', 'L', 'C', 'K' };
///
const uint16_t pluckValueAddress = pluckThresholdTagAddress + 4;
///
const uint16_t releaseThresholdTagAddress = pluckValueAddress + (numSensors * 2);
///
const char releaseTag[4] = { 'R', 'L', 'S', 'E' };
///
const uint16_t releaseValueAddress = releaseThresholdTagAddress + 4;
///
const uint16_t registerTypeTagAddress = releaseValueAddress + (numSensors * 2);
///
const char registerTypeTag[4] = { 'R', 'E', 'G', 'I' };
///
const uint16_t registerTypeAddress = registerTypeTagAddress + 4;
///
uint16_t defaultThreshold = 1000;
///
bool waitingForInput = false;
///
uint16_t singleThresholds[numSensors];
///
uint16_t pluckThresholds[numSensors];
///
uint16_t releaseThresholds[numSensors];
///
///
enum JackRegister {
  FRONT_REGISTER,
  BACK_REGISTER
};

JackRegister jackRegister = FRONT_REGISTER;
//-----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  // Save some power
  digitalWrite(PIN_ENABLE_I2C_PULLUP, LOW);
  digitalWrite(PIN_ENABLE_SENSORS_3V3, LOW);
  // digitalWrite(LED_PWR, LOW);

  /// init LEDs
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  if (!fram.begin(addrSizeInBytes)) {
    Serial.println("No SPI FRAM found ... check your connections\r\n");
    halt();
    // digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDB, LOW);
  }

  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);

  delay(3000);

  while (!readThresholdValues()) {
    editDataValues();
  }
}

//-----------------------------------------------------------------------------

void loop() {
  printFramValues();
  Serial.println("Input an option or power off the Arduino\r\n");
  editDataValues();
}

//-----------------------------------------------------------------------------

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
  Serial.println("Enter one of the following options:");
  Serial.println("- s:  set single threshold");
  Serial.println("- p:  set hysteretic pluck threshold");
  Serial.println("- r:  set hysteretic release threshold");
  Serial.println("- j:  set jack register threshold");
  // Serial.println("- f:  set default threshold of 1000");
  // Serial.println("- e:  set custom threshold (0-4096)");
}

//-----------------------------------------------------------------------------

void editDataValues() {

  printInputOptions();

  waitingForInput = true;

  while (waitingForInput) {
    if (Serial.available()) {

      char option = Serial.read();
      clearSerialInputBuffer();

      switch (option) {
        case 's':
        case 'S':
          setDefaultThreholdValue(0, 4095, singleThresholds);
          // (const char tag[4], const uint16_t tagAddr, int8_t *thresholdValues, uint16_t thresholdValueAddr)
          writeThresholdsToEEPROM(singleTag, singleThresholdTagAddress, (int8_t *)singleThresholds, thresholdValueAddress);
          break;
        case 'p':
        case 'P':
          setDefaultThreholdValue(0, 4095, pluckThresholds);
          writeThresholdsToEEPROM(pluckTag, pluckThresholdTagAddress, (int8_t *)pluckThresholds, pluckValueAddress);
          break;
        case 'r':
        case 'R':
          setDefaultThreholdValue(0, 4095, releaseThresholds);
          writeThresholdsToEEPROM(releaseTag, releaseThresholdTagAddress, (int8_t *)releaseThresholds, releaseValueAddress);
          break;
        case 'j':
        case 'J':
          setJackRegister();
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

bool readThresholdValues() {
  uint32_t tagRead;

  // Single Threshold
  fram.read(singleThresholdTagAddress, (uint8_t *)&tagRead, 4);

  bool allDataValid = true;

  if (tagRead != *((uint32_t *)singleTag)) {
    allDataValid = false;
    Serial.println("No valid single threshold data!");
  } else {
    fram.read(thresholdValueAddress, (uint8_t *)singleThresholds, numSensors * 2);
  }

  // Hysteresis Pluck Threshold
  fram.read(pluckThresholdTagAddress, (uint8_t *)&tagRead, 4);

  if (tagRead != *((uint32_t *)pluckTag)) {
    allDataValid = false;
    Serial.println("No valid hysteretic pluck threshold data!");
  } else {
    fram.read(pluckValueAddress, (uint8_t *)pluckThresholds, numSensors * 2);
  }

  // Hysteresis Release Threshold
  fram.read(releaseThresholdTagAddress, (uint8_t *)&tagRead, 4);

  if (tagRead != *((uint32_t *)releaseTag)) {
    allDataValid = false;
    Serial.println("No hysteretic release data!");
  } else {
    fram.read(releaseValueAddress, (uint8_t *)releaseThresholds, numSensors * 2);
  }

  // Register Select
  fram.read(registerTypeTagAddress, (uint8_t *)&tagRead, 4);

  if (tagRead != *((uint32_t *)registerTypeTag)) {
    allDataValid = false;
    Serial.println("No jack register data!");
  } else {
    fram.read(registerTypeAddress, (uint8_t *)(&jackRegister), sizeof(jackRegister));
  }

  return allDataValid;
}

void writeThresholdsToEEPROM(const char tag[4], const uint16_t tagAddr, int8_t *thresholdValues, uint16_t thresholdValueAddr) {

  fram.writeEnable(true);

  if (!fram.write(tagAddr, (uint8_t *)tag, 4))
    Serial.println("Failed to write tag");

  fram.writeEnable(true);

  if (!fram.write(thresholdValueAddr, (uint8_t *)thresholdValues, numSensors * 2))
    Serial.println("Failed to write data");

  fram.writeEnable(false);
}

//-----------------------------------------------------------------------------

void setDefaultThreholdValue(uint16_t min, uint16_t max, uint16_t *thresholdValues) {
  Serial.print("Enter a value (");
  Serial.print(min);
  Serial.print("-");
  Serial.print(max);
  Serial.println(") to initialise memory:");

  waitingForInput = true;

  while (waitingForInput) {
    if (Serial.available()) {
      defaultThreshold = Serial.parseInt(SKIP_WHITESPACE);
      clearSerialInputBuffer();
      Serial.print("Value Entered: ");
      Serial.println(defaultThreshold);
      if (defaultThreshold <= max && defaultThreshold >= min) {
        waitingForInput = false;
      } else {
        Serial.println("Enter a value (0-4095)");
      }
    }
  }
  for (int i = 0; i < numSensors; i++) {
    thresholdValues[i] = defaultThreshold;
  }
}

//-----------------------------------------------------------------------------

void setJackRegister() {
  Serial.println("Set the jack register. 1: front, 2: back");
  waitingForInput = true;
  while (waitingForInput) {

    if (Serial.available()) {

      char option = Serial.read();
      clearSerialInputBuffer();

      switch (option) {
        case '1':
          jackRegister = FRONT_REGISTER;
          waitingForInput = false;
          break;
        case '2':
          jackRegister = BACK_REGISTER;
          waitingForInput = false;
          break;
        default:
          Serial.print(option);
          Serial.println(" invalid option:");
          break;
      }
    }
  }
  fram.writeEnable(true);

  if (!fram.write(registerTypeTagAddress, (uint8_t *)registerTypeTag, 4))
    Serial.println("Failed to write tag");

  fram.writeEnable(true);

  if (!fram.write(registerTypeAddress, (uint8_t *)(&jackRegister), sizeof(jackRegister)))
    Serial.println("Failed to write data");

  fram.writeEnable(false);
}

//-----------------------------------------------------------------------------

void clearSerialInputBuffer() {
  while (Serial.available())
    Serial.read();
}

//

void printFramValues() {

  Serial.println("------ Single Thesholds ------");
  for (int i = 0; i < 7; i++) {
    Serial.print("------ Board ");
    Serial.print(i);
    Serial.println(" ------");
    for (int j = 0; j < 7; j++) {
      Serial.print(j);
      Serial.print(": ");
      Serial.print(singleThresholds[j]);
      Serial.print(",");
    }
    Serial.println(",");
  }

  Serial.println("");
  Serial.println("------ Hystertic Pluck ------");
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

  Serial.println("");
  Serial.println("------ Hystertic Release ------");
  for (int i = 0; i < 7; i++) {
    Serial.print("------ Board ");
    Serial.print(i);
    Serial.println(" ------");
    for (int j = 0; j < 7; j++) {
      Serial.print(j);
      Serial.print(": ");
      Serial.print(releaseThresholds[j]);
      Serial.print(",");
    }
    Serial.println(",");
  }

  Serial.println();
  Serial.print("Register: ");

  switch (jackRegister) {
    case FRONT_REGISTER:
      Serial.println("Front");
      break;
    case BACK_REGISTER:
      Serial.println("Back");
      break;
    default:
      Serial.println("Unknown");
      break;
  }

  Serial.println();
}