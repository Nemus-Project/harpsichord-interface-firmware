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
uint16_t pluckThresholds[numSensors];
///
uint16_t releaseThresholds[numSensors];
///
uint16_t* prevSensorReadings = sensorReadingsB;
///
uint16_t* currSensorReadings = sensorReadingsA;
///
uint16_t* tempPointer;
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
//-----------------------------------------------------------------------------
// LED Variables
///
const size_t ledPin = 9;
///
Adafruit_NeoPixel leds(numSensors, ledPin, NEO_GRB + NEO_KHZ800);
///
unsigned long now = 0;
///
static int step = 0;

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
const uint16_t tagAddress = 0;
///
const uint8_t thresholdTag[4] = { 'D', 'A', 'T', 'A' };
// const uint8_t maxTag[4] = { 'M', 'A', 'X', 'I' };
// const uint8_t minTag[4] = { 'M', 'I', 'N', 'I' };
///
const uint16_t pluckValAddress = tagAddress + 4;
//-----------------------------------------------------------------------------
// MIDI Variables
/// MIDI Communication over USB Object, see the PluggableUSBMIDI library
USBMIDI MidiUSB;
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
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  leds.begin();
  leds.clear();

  analogReadResolution(12);

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
    if (currSensorReadings[i] < pluckThresholds[i] and prevSensorReadings[i] > pluckThresholds[i]) {
      noteOff(0, index2note(i), 100);
    } else if (currSensorReadings[i] > pluckThresholds[i] and prevSensorReadings[i] < pluckThresholds[i]) {
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
