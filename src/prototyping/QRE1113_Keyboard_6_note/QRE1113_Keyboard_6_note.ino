/** 
 * @file 
 * @brief QRE1113  6-Note Keyboard
 *  
 *  Firmware for 3-key, 6-jack Harpsochord model
 *
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
 */
//-----------------------------------------------------------------------------
#include <MIDIUSB.h>
#include <Rotary.h>
#include <Button2.h>
#include <FlashStorage_SAMD.h>
//-----------------------------------------------------------------------------

enum JackState {
  PLUCKED,
  PLUCK,
  PRESSING,
  RELEASING,
  RELEASED,
  UNKNOWN_KEY_STATE
};

//-----------------------------------------------------------------------------
// Sensor variables
const byte numSensors = 6;
unsigned long sensorAverages[numSensors];
unsigned long sensorHomeValues[numSensors];
const pin_size_t sensorPins[numSensors] = { A0, A1, A2, A3, A6, A7 };
const byte ledPins[numSensors] = { 3, 5, 7, 2, 4, 6 };
const bool ledFeedback = true;
int sensorValues[numSensors] = { 0, 0, 0, 0, 0, 0 };
int prevValues[numSensors] = { 0, 0, 0, 0, 0, 0 };
byte noteMap[numSensors] = { 70, 71, 72, 70, 71, 72 };
byte pluckValues[numSensors] = { 65, 80, 180, 80, 77, 36 };
byte releaseValues[numSensors] = { 24, 24, 24, 24, 24, 24 };
JackState jackStates[numSensors] = { RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED };
JackState prevStates[numSensors] = { RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED };
//-----------------------------------------------------------------------------
// Rotary Variables
const byte BUTTON_PIN = 10;
const byte ROTARY_PIN1 = 11;
const byte ROTARY_PIN2 = 12;
const byte CLICKS_PER_STEP = 4;
const byte MIN_POS = 0;
const byte MAX_POS = numSensors - 1;
const byte START_POS = 0;
const byte INCREMENT = 1;
Rotary r = Rotary(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
Button2 b = Button2(BUTTON_PIN);
byte curKey = 0;
bool isKeySelectMode = true;
bool executeDebugMode = false;
//-----------------------------------------------------------------------------
// EEPROM Vars
const uint16_t tagAddress = 0;
const char tagName[4] = { 'D', 'A', 'T', 'A' };
const uint16_t pluckValAddress = tagAddress + 4;
//-----------------------------------------------------------------------------

void setup() {
  analogReference(AR_INTERNAL1V65);
  Serial.begin(9600);
  calibrate();
  readPluckFromEEPROM();
  for (auto pin : ledPins) {
    pinMode(pin, OUTPUT);
  }
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
  // printReadings();
  checkJackStates();
  printJackStates();
  delay(1);
}

//-----------------------------------------------------------------------------