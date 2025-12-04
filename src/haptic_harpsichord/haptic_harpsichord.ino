/**
 * @dir ./src/haptic_harpsichord
 * @brief Final Firmware for NEMUS harpsichord interface
 *
 * @file haptic_harpsichord.ino
 * @brief Main firmware file for Arduino IDE
 *
 * 
 * This is the full firmware for the haptic harpsochord using the arduino IDE development platform
 *
 * At time of writing the microcontroller for the project is not fixed. As such there may be macros to help make the correct changes to firmware behaviour.
 *
 * ## Pinout
 *
 * |     Connection | Pin Left | Pin Right | Connection      |
 * | -------------: | :------- | --------: | :-------------- |
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
 * @date 2025-10-11
 * @author Matthew Hamilton
 *
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
///
enum JackState {
  PLUCKED,
  PLUCK,
  PRESSING,
  RELEASING,
  RELEASED,
  UNKNOWN_KEY_STATE
};

///
enum JackRegister {
  FRONT_REGISTER,
  BACK_REGISTER
};

///
enum RotaryMode {
  KEY_SELECT,
  EDIT_SINGLE_THRESHOLD,
  EDIT_PLUCK_THRESHOLD,
  EDIT_RELEASE_THRESHOLD,
  REGISTER_SELECT,
};

///
enum ThresholdType {
  SINGLE_THRESHOLD,
  HYSTERETIC,
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
byte index2note(byte index, int8_t transpose = 0);
void rotate(Rotary& r);
//-----------------------------------------------------------------------------
// Config Variables
/// Standard Baud Rate for MIDI
const unsigned long midiBaudRate = 115200;  // 31250;
/// Flag to use in `setup` to enter debug mode
bool executeDebugMode = false;
//-----------------------------------------------------------------------------
// Operation Mode Variables
/// Debug Mode Rotary behaviour flag
bool isKeySelectMode = true;
/// Debug mode serial plot key index
uint8_t curKeyIndex = key2index(47);
//-----------------------------------------------------------------------------
// Multiplexer Variables
/// Mux Channel Set Pin A
const size_t muxPinA = 6;
/// Mux Channel Set Pin B
const size_t muxPinB = 7;
/// Mux Channel Set Pin C
const size_t muxPinC = 8;
// const size_t muxPinD = 9;
//-----------------------------------------------------------------------------
// Sensor variables
///
uint16_t sensorReadingsA[numSensors];
///
uint16_t sensorReadingsB[numSensors];
///
uint16_t singlePluckThresholds[numSensors];
///
uint16_t pluckThresholds[numSensors];
///
uint16_t releaseThresholds[numSensors];
///
uint16_t* prevSensorReadings = sensorReadingsB;
///
uint16_t* currSensorReadings = sensorReadingsA;
///
uint16_t* tempPointer;
///
ThresholdType thresholdType = SINGLE_THRESHOLD;
//-----------------------------------------------------------------------------
// Jack States
///
JackState jackStatesA[numSensors];
///
JackState jackStatesB[numSensors];
///
JackState* jackStates = jackStatesA;
///
JackState* prevStates = jackStatesB;
///
JackState* tempStatePointer;
///
JackRegister jackRegister = FRONT_REGISTER;
//-----------------------------------------------------------------------------
// LED Variables
///
const size_t ledPin = 9;
///
Adafruit_NeoPixel leds(numSensors, ledPin, NEO_GRB + NEO_KHZ800);
/// used during animation for current millis time
unsigned long now = 0;
/// step size for animation: static to avoid name collision
static int step = 0;
///
const uint32_t keySelectColor = leds.Color(0, 0, 100);
///
const uint32_t editPluckThresholdColor = leds.Color(0, 100, 0);
///
const uint32_t editReleaseThresholdColor = leds.Color(100, 0, 100);
///
uint32_t currentLedColor = keySelectColor;
//-----------------------------------------------------------------------------
// Rotary Variables
///
const byte ROTARY_PINC = 12;
///
const byte ROTARY_PINA = 10;
///
const byte ROTARY_PINB = 11;
///
const byte CLICKS_PER_STEP = 4;
///
const byte MIN_POS = 0;
///
const uint16_t MAX_POS = numSensors - 1;
///
const byte START_POS = 0;
///
const byte INCREMENT = 1;
///
Rotary rotary = Rotary(ROTARY_PINA, ROTARY_PINB, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
///
Button2 button = Button2(ROTARY_PINC);
///
RotaryMode rotaryMode = KEY_SELECT;
//-----------------------------------------------------------------------------
// EEPROM Variables
///
const uint8_t FRAM_CS = 2;
///
const uint8_t FRAM_MOSI = 3;
///
const uint8_t FRAM_MISO = 4;
///
const uint8_t FRAM_SCK = 5;
///
Adafruit_FRAM_SPI fram = Adafruit_FRAM_SPI(FRAM_SCK, FRAM_MISO, FRAM_MOSI, FRAM_CS);
///
const uint8_t addrSizeInBytes = 2;  // Default to address size of two bytes
///
const uint16_t singleThresholdTagAddress = 0;
///
const uint8_t singleThresholdTag[4] = { 'D', 'A', 'T', 'A' };
///
const uint16_t thresholdValueAddress = singleThresholdTagAddress + 4;
///
const uint16_t pluckThresholdTagAddress = thresholdValueAddress + (numSensors * 2);
///
const uint8_t pluckTag[4] = { 'P', 'L', 'C', 'K' };
///
const uint16_t pluckValueAddress = pluckThresholdTagAddress + 4;
///
const uint16_t releaseThresholdTagAddress = pluckValueAddress + (numSensors * 2);
///
const uint8_t releaseTag[4] = { 'R', 'L', 'S', 'E' };
///
const uint16_t releaseValueAddress = releaseThresholdTagAddress + 4;
///
const uint16_t registerTypeTagAddress = releaseValueAddress + (numSensors * 2);
///
const uint8_t registerTypeTag[4] = { 'R', 'E', 'G', 'I' };
///
const uint16_t registerTypeAddress = releaseValueAddress + (numSensors * 2);
//-----------------------------------------------------------------------------
// MIDI Variables
/// MIDI Communication over USB Object, see the PluggableUSBMIDI library
USBMIDI MidiUSB;
/// sensor index to note table for the front register
byte frontRegisterNoteTable[numSensors] = {94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46};
/// sensor index to note table for the back register
byte backRegisterNoteTable[numSensors]  = {46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94};
//-----------------------------------------------------------------------------
// Misc
/// Use when waiting for user input from the serial port or rotary encoder
bool waitingForInput = false;
//-----------------------------------------------------------------------------

/**
 * @brief Arduino setup function
 * 
 * Initalises all peripheral components and listen if the rotary encoder
 * button has been pressed to enter debug mode.
 */
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

  leds.begin();
  leds.clear();

  analogReadResolution(12);

  rotary.setChangedHandler(rotate);
  button.setDoubleClickHandler(doubleclick);
  button.setClickHandler(click);

  /// setup EEPROM
  if (!fram.begin())
    halt(FRAM_NOT_FOUND);

  readPluckFromEEPROM();

  if (button.isPressed() or ALWAYS_DEBUG) {
    debugLoop();
  }
}

/**
 * @brief Main firmware loop
 * Listen to sensor values and generate MIDI one those thresholds have been crossed
 */
void loop() {

  readSensors();

  for (int i = 0; i < numSensors; i++) {
    if (currSensorReadings[i] < singlePluckThresholds[i] and prevSensorReadings[i] > singlePluckThresholds[i]) {
      noteOff(0, index2note(i), 100);
    } else if (currSensorReadings[i] > singlePluckThresholds[i] and prevSensorReadings[i] < singlePluckThresholds[i]) {
      noteOn(0, index2note(i), 100);
    }
  }
}

/**
 * @brief Translate from key index to key number
 * 
 * @param i key index 
 * @return key number as it appears on the interface
 */
constexpr byte index2key(byte i) {
  return (numSensors - i);
}

/**
 * @brief Key number to keyindex
 * 
 * @param k Key number on interface
 * @return corrresponding key index used in threshold arrays
 */
constexpr byte key2index(byte k) {
  if (k > numSensors)
    k = numSensors;
  else if (k < 1)
    k = 1;
  return (numSensors - k);
}

/**
 * @brief Set the jack register type
 * 
 * Waits for user input via the serial console or the rotary encoder
 *
 * @see JackRegister
 */
void setRegister() {
  Serial.println("Set the jack register. 1: front, 2: back");

  waitingForInput = true;
  rotaryMode = REGISTER_SELECT;

  while (waitingForInput) {
    rotary.loop();

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
          Serial.println("invalid option:");
          break;
      }
    }
  }
}

/**
 * @brief Clears the Serial Input Buffer
 * 
 * Given the Serial input options are relativley simple, this functions 
 * can be use to clear the input buffer and avoid mistypes
 *
 */
void clearSerialInputBuffer() {
  while (Serial.available())
    Serial.read();
}