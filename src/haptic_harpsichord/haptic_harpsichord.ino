/*
  Haptic Harpsichord

  This is the full firmware for the haptic harpsochord using the arduino IDE development platform

  At time of writing the microcontroller for the project is not fixed. As such there may be macros to help make the correct changes to firmware behaviour.

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
// Board specific libraries
#ifdef ARDUINO_ARDUINO_NANO33BLE
#include <PluggableUSBMIDI.h>
#elif ESP32
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>  // verision 3.1.3
#include <MIDI.h>
#elif ARDUINO_ARCH_STM32
#endif
//-----------------------------------------------------------------------------
// Board Agnostics Libraries
#include <Rotary.h>
#include <Button2.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_FRAM_SPI.h>
#include <SPI.h>
//-----------------------------------------------------------------------------
// Local Headers
#include "error_codes.h"
//-----------------------------------------------------------------------------
#define ALWAYS_DEBUG true
//-----------------------------------------------------------------------------
// Enums
enum JackState {
  PLUCKED,
  PLUCK,
  PRESSING,
  RELEASING,
  RELEASED,
  UNKNOWN_KEY_STATE
};
//-----------------------------------------------------------------------------
#define numSensors 49
#define numMuxChannels 7
#define numPcbs 7

#if (numMuxChannels * numPcbs != numSensors)
#error "Check product of mux channels and number of PCBs is equal to total number of sensors"
#endif
//-----------------------------------------------------------------------------
constexpr byte key2index(byte k);
byte index2note(byte index, byte transpose = 0);
void rotate(Rotary& r);
//-----------------------------------------------------------------------------
// Config Variables
const unsigned long midiBaudRate = 115200;  // 31250;
bool executeDebugMode = false;
//-----------------------------------------------------------------------------
// Operation Mode Variables
bool isKeySelectMode = true;
uint8_t curKeyIndex = key2index(25);
//-----------------------------------------------------------------------------
// Multiplexer Variables
const size_t muxPinA = 6;
const size_t muxPinB = 7;
const size_t muxPinC = 8;
// const size_t muxPinD = 9;
//-----------------------------------------------------------------------------
// Sensor variables
uint16_t sensorReadingsA[numSensors];
uint16_t sensorReadingsB[numSensors];
// uint16_t lastSensorReading[numSensors];
uint16_t* tempPointer;
uint16_t sensorAvgMaxima[numSensors];
uint16_t sensorAvgMinima[numSensors];
uint16_t pluckThresholds[numSensors];
uint16_t releaseThresholds[numSensors];
uint32_t readCount = 0;
uint64_t lastRead = 0;
constexpr byte avgSize = 4;
uint16_t sensorWindowReadings[numSensors][avgSize];
byte windex = 0;
uint16_t* prevSensorReadings = sensorReadingsB;
uint16_t* currSensorReadings = sensorReadingsA;
//-----------------------------------------------------------------------------
// Jack States
JackState jackStatesA[numSensors];
JackState jackStatesB[numSensors];
JackState* jackStates = jackStatesA;
JackState* prevStates = jackStatesB;
JackState* tempStatePointer;
//-----------------------------------------------------------------------------
// LED Variables
const size_t ledPin = 9;
Adafruit_NeoPixel leds(numSensors, ledPin, NEO_GRB + NEO_KHZ800);
unsigned long now = 0;
static int step = 0;

//-----------------------------------------------------------------------------
// Rotary Variables
const byte ROTARY_PINC = 12;
const byte ROTARY_PINA = 10;
const byte ROTARY_PINB = 11;
const byte CLICKS_PER_STEP = 4;
const byte MIN_POS = 0;
const uint16_t MAX_POS = numSensors - 1;
const byte START_POS = 0;
const byte INCREMENT = 1;
Rotary rotary = Rotary(ROTARY_PINA, ROTARY_PINB, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
Button2 button = Button2(ROTARY_PINC);
//-----------------------------------------------------------------------------
// EEPROM Variables
const uint8_t FRAM_CS = 2;
const uint8_t FRAM_MOSI = 3;
const uint8_t FRAM_MISO = 4;
const uint8_t FRAM_SCK = 5;
Adafruit_FRAM_SPI fram = Adafruit_FRAM_SPI(FRAM_SCK, FRAM_MISO, FRAM_MOSI, FRAM_CS);
const uint8_t addrSizeInBytes = 2;  // Default to address size of two bytes
const uint16_t tagAddress = 0;
const uint8_t thresholdTag[4] = { 'D', 'A', 'T', 'A' };
// const uint8_t maxTag[4] = { 'M', 'A', 'X', 'I' };
// const uint8_t minTag[4] = { 'M', 'I', 'N', 'I' };
const uint16_t pluckValAddress = tagAddress + 4;

unsigned long times[1024] = { 0 };
//-----------------------------------------------------------------------------
// MIDI Variables
USBMIDI MidiUSB;

void setup() {
  Serial.begin(midiBaudRate);
  // Save some power
  digitalWrite(PIN_ENABLE_I2C_PULLUP, LOW);
  digitalWrite(PIN_ENABLE_SENSORS_3V3, LOW);
  // digitalWrite(LED_PWR, LOW);
  /// setup mux
  pinMode(muxPinA, OUTPUT);
  pinMode(muxPinB, OUTPUT);
  pinMode(muxPinC, OUTPUT);

  /// init LEDs
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  for (int i = 0; i < numSensors; i++) {
    sensorAvgMinima[i] = 1024;
    pluckThresholds[i] = 500;
  }

  leds.begin();
  leds.clear();

  // /// setup EEPROM
  // if (!fram.begin())
  //   halt(FRAM_NOT_FOUND);

  // readPluckFromEEPROM();

  // calibrarte sensors
  // calibrate();

  if (button.isPressed() or ALWAYS_DEBUG) {
    debugLoop();
  }
}

void loop() {
  readSensors();

  for (int i = 0; i < numSensors; i++) {
    if (currSensorReadings[i] < pluckThresholds[i] and prevSensorReadings[i] > pluckThresholds[i]) {
      noteOff(0, index2note(i), 100);
    } else if (currSensorReadings[i] > pluckThresholds[i] and prevSensorReadings[i] < pluckThresholds[i]) {
      noteOn(0, index2note(i), 100);
    }
  }
}

void calibrate() {
  static const int numReading = 8;

  for (int i = 0; i < numSensors; i++) {
    unsigned long runningTotal = 0;

    for (int k = 0; k < numReading; k++) {
      runningTotal += readSensor(i);
    }
    sensorAvgMaxima[i] = runningTotal /= numReading;
  }
}

constexpr byte index2key(byte i) {
  return (numSensors - i);
}

constexpr byte key2index(byte k) {
  if (k > numSensors)
    k = numSensors;
  else if (k < 1)
    k = 1;
  return (numSensors - k);
}
