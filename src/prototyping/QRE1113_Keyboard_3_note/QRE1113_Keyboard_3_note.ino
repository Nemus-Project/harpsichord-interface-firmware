/*
  QRE1113  3-Note Keyboard
  
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