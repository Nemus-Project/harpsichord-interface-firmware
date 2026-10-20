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
#define FIRMWARE_STRING "0.2.2"
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
#define ALWAYS_DEBUG false
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

/// Note detection method. Use HYSTERETIC.
///
/// @warning SINGLE_THRESHOLD is DEPRECATED and must not be used. It is kept
/// for reference only and is not maintained: it has no jack-state gating,
/// its debug loop ignores serial commands (no auto-calibration, no register
/// change), and it is not tested on the current hardware.
enum ThresholdType {
  SINGLE_THRESHOLD,  ///< @deprecated do not use, see ThresholdType
  HYSTERETIC,
};
//-----------------------------------------------------------------------------
#define numSensors 49
#define numMuxChannels 7
#define numPcbs 7

// Orphan sensors: indices 0-1 and 47-48 have no key above them (the sensor
// rail was installed with 4 extra positions by mistake). The main loops
// iterate only over the active range [FIRST_ACTIVE_SENSOR..LAST_ACTIVE_SENSOR].
#define FIRST_ACTIVE_SENSOR 2
#define LAST_ACTIVE_SENSOR  46

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
uint16_t singlePluckThresholds[numSensors];
///
uint16_t pluckThresholds[numSensors];
///
uint16_t releaseThresholds[numSensors];
///
uint16_t sensorReadingsA[numSensors];
///
uint16_t sensorReadingsB[numSensors];
///
uint16_t* prevSensorReadings = sensorReadingsB;
///
uint16_t* currSensorReadings = sensorReadingsA;
///
uint16_t* tempPointer;
///
ThresholdType thresholdType = HYSTERETIC;  // must stay HYSTERETIC: SINGLE_THRESHOLD is deprecated
//-----------------------------------------------------------------------------
// Moving average filter
/// Window length — must be a power of 2 so division becomes a bit shift
#define AVG_SIZE 8
#define AVG_SHIFT 3   // log2(AVG_SIZE) — update if you change AVG_SIZE
/// Largest raw ADC value (see `analogReadResolution(12)` in `setup()`)
#define ADC_MAX 4095
/// Largest filtered reading: AVG_SIZE samples summed then shifted by
/// AVG_SHIFT - 1, i.e. 13-bit (8190). Use this, not 4096, for reading ranges.
#define SENSOR_READING_MAX ((ADC_MAX * AVG_SIZE) >> (AVG_SHIFT - 1))
/// Threshold written to FRAM when no calibration is stored (13-bit scale)
#define DEFAULT_THRESHOLD 1600
/// Reading change per scan that maps to the top of the velocity range (13-bit scale)
#define VELOCITY_DELTA_MAX 360
/// Per-sensor ring buffer of raw samples
uint16_t sensorHistory[numSensors][AVG_SIZE];
/// Per-sensor running sum (wide enough for 16-bit samples × AVG_SIZE entries)
uint32_t sensorSums[numSensors];
/// Circular index into sensorHistory
uint8_t windex = 0;
//-----------------------------------------------------------------------------
// Sensor velocity (updated every scan by readSensors())
/// Rate of change of each reading, in reading units (13-bit scale) per second.
/// Negative = reading falling (key going down), positive = rising.
/// It is the average slope over the moving-average window: (newest raw sample
/// - raw sample AVG_SIZE scans ago) / time between those two scans.
float sensorVelocity[numSensors];
/// micros() timestamp of each scan in the window, indexed like sensorHistory
unsigned long scanTimes[AVG_SIZE];
/// Measured time between the last two scans, in microseconds (diagnostic)
unsigned long scanDtMicros = 0;
/// Measured time spanned by the window (AVG_SIZE scans), in microseconds
unsigned long windowMicros = 0;
/// Number of scans done, saturating at AVG_SIZE: velocity is valid once the
/// window is full
uint8_t scanCount = 0;
/// Serial Plotter velocity view: fixed plot range ±VELOCITY_PLOT_RANGE
/// (reading units per second)
#define VELOCITY_PLOT_RANGE 20000
//-----------------------------------------------------------------------------
// Jack States
///
JackState jackStatesA[numSensors];
///
JackState jackStatesB[numSensors];
///
JackState jackStates[numSensors];
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
/// How long the encoder button must be held to toggle play / calibration mode
const unsigned int MODE_SWITCH_HOLD_MS = 2000;
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
const uint16_t registerTypeAddress = registerTypeTagAddress + 4;
//-----------------------------------------------------------------------------
// MIDI Variables
/// MIDI Communication over USB Object, see the PluggableUSBMIDI library
USBMIDI MidiUSB;
//
// Sensor-to-MIDI-note mapping.
//
// Active sensors are indices 2-46, mapped chromatically across E2 (40) to
// C6 (84) — 45 notes total. Indices 0-1 and 47-48 are orphans (no key above
// them); their entries are 0 placeholders that the main loops skip via
// FIRST_ACTIVE_SENSOR / LAST_ACTIVE_SENSOR. The orphans sit symmetrically at
// both ends, so the active range is the same for both registers.
//
// The sensor rail reads the keys in opposite order depending on which jack
// register it sits under, so the two tables are mirror images:
//   back register:  index 2 = E2 ... index 46 = C6 (ascending)
//   front register: index 2 = C6 ... index 46 = E2 (descending)
// The register is stored in FRAM; change it with `f` / `b` over serial in
// debug mode (see changeJackRegister()).
//
/// sensor index to note table for the front register
byte frontRegisterNoteTable[numSensors] = {
   0,  0,                                            // orphans
  84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73,    // C6  B5  Bb5 A5  G#5 G5  F#5 F5  E5  D#5 D5  C#5
  72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61,    // C5  B4  Bb4 A4  G#4 G4  F#4 F4  E4  D#4 D4  C#4
  60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,    // C4  B3  Bb3 A3  G#3 G3  F#3 F3  E3  D#3 D3  C#3
  48, 47, 46, 45, 44, 43, 42, 41, 40,                // C3  B2  Bb2 A2  G#2 G2  F#2 F2  E2
   0,  0                                             // orphans
};
/// sensor index to note table for the back register
byte backRegisterNoteTable[numSensors] = {
   0,  0,                                            // orphans
  40, 41, 42, 43, 44, 45, 46, 47, 48,                // E2  F2  F#2 G2  G#2 A2  Bb2 B2  C3
  49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,    // C#3 D3  D#3 E3  F3  F#3 G3  G#3 A3  Bb3 B3  C4
  61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,    // C#4 D4  D#4 E4  F4  F#4 G4  G#4 A4  Bb4 B4  C5
  73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,    // C#5 D5  D#5 E5  F5  F#5 G5  G#5 A5  Bb5 B5  C6
   0,  0                                             // orphans
};
//-----------------------------------------------------------------------------
// Misc
/// Use when waiting for user input from the serial port or rotary encoder
bool waitingForInput = false;
/// boolean for printing a data stream to calibrate
bool shouldPrint = true;
/// Serial Plotter view in debug mode: false = reading + thresholds, true = velocity
bool plotVelocity = false;
//-----------------------------------------------------------------------------

/**
 * @brief Arduino setup function
 * 
 * Initalises all peripheral components and listen if the rotary encoder
 * button has been pressed to enter debug mode.
 */
void setup() {
  Serial.begin(midiBaudRate);

  for (int i = 0; i < numSensors; i++) {
    jackStates[i] = RELEASED;
  }
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
  button.setLongClickTime(MODE_SWITCH_HOLD_MS);
  button.setLongClickDetectedHandler(longClickDetected);

  /// setup EEPROM
  if (!fram.begin())
    halt(FRAM_NOT_FOUND);

  delay(3000);
  readPluckFromEEPROM();

  // Prime the moving-average ring buffer so the first few scans aren't
  // biased by the zero-initialised history.
  for (uint8_t i = 0; i < AVG_SIZE; i++) {
    readSensors();
  }

  // Hold the encoder button during boot to start in debug (calibration) mode.
  // isPressedRaw() reads the pin now; isPressed() only reports the state
  // captured when the global `button` was constructed at power-on.
  if (button.isPressedRaw() or ALWAYS_DEBUG) {
    executeDebugMode = true;

    // Wait for release, then resync Button2's internal state. Otherwise the
    // boot press is seen as a long press and immediately exits debug mode.
    while (button.isPressedRaw())
      delay(10);
    delay(50);
    button.begin(ROTARY_PINC);
  }

  printFirmwareInfo();
}

/**
 * @brief Main firmware loop
 * Listen to sensor values and generate MIDI one those thresholds have been crossed
 *
 * Long-pressing the encoder button toggles `executeDebugMode`, which makes the
 * current play or debug loop return so the other one is entered here.
 */
void loop() {
  if (executeDebugMode) {
    debugLoop();
    return;
  }

  switch (thresholdType) {
    case SINGLE_THRESHOLD:
      singleThresholdLoop();
      break;
    case HYSTERETIC:
      hysteresisLoop();
      break;
  }
}

/**
 * @brief Play loop for single-threshold detection
 *
 * @deprecated DO NOT USE. Kept for reference only, see ThresholdType.
 */
void singleThresholdLoop() {
  while (!executeDebugMode) {
    readSensors();
    button.loop();
    processSingleThreshold();
  }
}

void hysteresisLoop() {
  while (!executeDebugMode) {
    readSensors();
    button.loop();
    processHysteretic();
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
 * @deprecated Not called any more. It blocked boot until a serial answer
 * arrived, and the encoder click could not confirm. A fresh FRAM now defaults
 * to BACK_REGISTER (see readPluckFromEEPROM()), and the register is changed
 * with `f` / `b` in debug mode (changeJackRegister()).
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
 * @brief Switch the jack register at runtime and save it to FRAM
 *
 * Any note currently sounding is released first, using the old mapping, so
 * no note is left hanging when the sensor-to-note table changes.
 *
 * @param newRegister register to switch to
 * @see JackRegister
 */
void changeJackRegister(JackRegister newRegister) {
  if (newRegister != jackRegister) {
    for (int i = FIRST_ACTIVE_SENSOR; i <= LAST_ACTIVE_SENSOR; i++) {
      if (jackStates[i] == PLUCK) {
        noteOff(0, index2note(i), 100);
        jackStates[i] = RELEASED;
      }
    }
    jackRegister = newRegister;
    writeRegisterToEEPROM();
  }

  Serial.print("Jack Register: ");
  Serial.println(jackRegister == FRONT_REGISTER ? "Front" : "Back");
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
