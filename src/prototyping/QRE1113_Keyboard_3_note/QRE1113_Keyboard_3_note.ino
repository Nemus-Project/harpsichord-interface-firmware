/**
 *  @file QRE1113_Keyboard_3_note.ino
 *  @brief QRE1113  3-Note Keyboard
 *  
 * Firmware for 3-key, 6-jack Harpsochord model
 *
 *  ## Wiring
 *
 * | Connection     | Pin Left | Pin Right | Connection    |
 * | -------------- | -------- | --------- | ------------- |
 * | NC             | D13      | D12       | Rotary CLK    |
 * | NC             | 3.3v     | D11       | Rotary DATA   |
 * | NC             | AREF     | D10       | Rotary Switch |
 * | Jack 0 QRE1113 | A0       | D9        | NC            |
 * | Jack 1 QRE1113 | A1       | D8        | NC            |
 * | Jack 2 QRE1113 | A2       | D7        | Jack 5 LED    |
 * | Jack 3 QRE1113 | A3       | D6        | Jack 4 LED    |
 * | NC             | A4       | D5        | Jack 3 LED    |
 * | NC             | A5       | D4        | Jack 2 LED    |
 * | Jack 4 QRE1113 | A6       | D3        | Jack 1 LED    |
 * | Jack 5 QRE1113 | A7       | D2        | Jack 0 LED    |
 * | NC             | 5v       | GND       | GND           |
 * | NC             | RESET    | RST       | NC            |
 * | GND            | GND      | RX / D1   | NC            |
 * | NC             | VIN      | TX / D0   | NC            |
 *
 *
 *  ## Jack Layout
 *
 * ```
 *    ┌─┐┌─┐┌─┐
 *    ┤3│┤4│┤5│
 *    └─┘└─┘└─┘
 *    ┌─┐┌─┐┌─┐
 *    │0├│1├│2├
 *    └─┘└─┘└─┘
 *
 *    ┌──┬─┬──┐
 *    │  │ │  │
 *    │  │ │  │
 *    │  │ │  │
 *    │  └┬┘  │
 *    │   │   │
 *    │   │   │
 *    └───┴───┘
 * ```
 * @date 2025-10-11
 * @author Matthew Hamilton
 */
//-----------------------------------------------------------------------------

// #include <MIDIUSB.h>
#include <PluggableUSBMIDI.h>
#include <Rotary.h>
#include <Button2.h>
#include <FlashStorage_SAMD.h>
//-----------------------------------------------------------------------------
USBMIDI MidiUSB;

enum MIDI_EVENT_STATUS {
  NOTE_OFF = 0x8,
  NOTE_ON = 0x9,
  POLY_AFTERTOUCH = 0xA,
  CONTROL_CHANGE = 0xB,
  PROGRAM_CHANGE = 0xC,
  AFTERTOUCH = 0xD,
  PITCH_WHEEL = 0xE,
};

enum JackState {
  PLUCKED,
  PLUCK,
  PRESSING,
  RELEASING,
  RELEASED,
  UNKNOWN_KEY_STATE
};

//-----------------------------------------------------------------------------
// Rotary Variables
const byte BUTTON_PIN = 10;
const byte ROTARY_PIN1 = 11;
const byte ROTARY_PIN2 = 12;
const byte CLICKS_PER_STEP = 4;
const byte MIN_POS = 0;
const byte MAX_POS = 2;
const byte START_POS = 0;
const byte INCREMENT = 1;
Rotary r = Rotary(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
Button2 b = Button2(BUTTON_PIN);
const byte ledPins[3] = { 2, 3, 5 };
byte curKey = 0;
bool isKeySelectMode = true;
bool executeDebugMode = false;
//-----------------------------------------------------------------------------
// EEPROM Vars
const uint16_t tagAddress = 0;
const char tagName[4] = { 'D', 'A', 'T', 'A' };
const uint16_t pluckValAddress = tagAddress + 4;
//-----------------------------------------------------------------------------
// Sensor variables
unsigned long sensorAverages[3] = { 0, 0, 0 };
unsigned long sensorHomeValues[3] = { 0, 0, 0 };
const int sensorPins[3] = { A2, A1, A0 };
int sensorValues[3] = { 0, 0, 0 };
int prevValues[3] = { 0, 0, 0 };
byte pluckValues[3] = { 158, 57, 61 };
byte releaseValues[3] = { 24, 24, 24 };
unsigned long currentTime[3] = { 0, 0, 0 };
unsigned long previousTime[3] = { 0, 0, 0 };
JackState jackStates[3];
JackState prevStates[3];

//-----------------------------------------------------------------------------

void setup() {
  analogReference(AR_INTERNAL1V65);
  Serial.begin(9600);
  calibrate();
  readPluckFromEEPROM();

  delay(200);  // give some padding to allow button to be pressed

  if (b.isPressed()) {

    setupDebugMode();

    while (executeDebugMode)
      debugModeLoop();

    exitDebug();
  }
}

void loop() {
  updateSensorReadings();
  updateJackStates();
  printReadings();
  checkJackStates();
  // printJackStates();
  delay(1);
}

//-----------------------------------------------------------------------------