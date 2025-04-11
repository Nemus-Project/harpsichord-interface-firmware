
/*
  One Key Firmware

  Read optical and pressure sensors to trigger MIDI data for 
  NEMUS Non-Linear String Model

  The pluckThreshold and releaseThreshold provide a level of hysteresis to
  stop notes from being triggered unnecessarily.
 */
#include <PluggableUSBMIDI.h>
//-----------------------------------------------------------------------------

enum OPMODE { NORMAL,
              DEBUG_MODE,
              EMULATE,
};

enum KEYSTATE {
  RELEASED,
  PRESSED,
  PLUCKED,
};
//-----------------------------------------------------------------------------
const int numSensors = 2;
const int sensorPins[numSensors] = { A3, A1 };
uint16_t sensorValues[numSensors] = { 0, 0 };
int prevValues[numSensors] = { 0, 0 };
int pluckThresholds[numSensors] = { 230, 610 };
const int releaseThresholds[numSensors] = { 80, 550 };
const int noteMap[numSensors] = { 36, 36 };
const int chanMap[numSensors] = { 0, 1 };
//-----------------------------------------------------------------------------
unsigned long currentTime[2] = { 0, 0 };
unsigned long previousTime[2] = { 0, 0 };
KEYSTATE keyStates[2] = { RELEASED, RELEASED };
//-----------------------------------------------------------------------------
const size_t dataLogSize = 16384;
uint16_t datalogIndex = 0;
uint16_t sensorLog1[dataLogSize];
uint16_t sensorLog2[dataLogSize];
//-----------------------------------------------------------------------------
USBMIDI MidiUSB;
//-----------------------------------------------------------------------------
OPMODE state = DEBUG_MODE;
KEYSTATE key_state = RELEASED;
//-----------------------------------------------------------------------------
void noteOn(byte channel, byte pitch, byte velocity) {
  MidiUSB.write(MIDIMessage::NoteOn(pitch, velocity, channel));
}

void noteOff(byte channel, byte pitch, byte velocity) {
  MidiUSB.write(MIDIMessage::NoteOff(pitch, velocity, channel));
}
//-----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // analogReadResolution(12);
}
//-----------------------------------------------------------------------------
void loop() {

  updateSensorReadings();

  switch (state) {
    case NORMAL:
      for (int i = 0; i < numSensors; i++) {
        if (prevValues[i] <= pluckThresholds[i] && sensorValues[i] > pluckThresholds[i] && keyStates[i] != PLUCKED) {
          velocity = sensorValues[i] - prevValues[i];
          noteOn(chanMap[i], noteMap[i], velocity);
          keyStates[i] = PLUCKED;
        } else if (prevValues[i] >= releaseThresholds[i] && sensorValues[i] < releaseThresholds[i] && keyStates[i] == PLUCKED) {
          velocity = prevValues[i] - sensorValues[i];
          noteOff(chanMap[i], noteMap[i], velocity);
          keyStates[i] = PRESSED;
        }
      }
      break;
    case DEBUG_MODE:
      printReadings();
      break;
    case EMULATE:

      noteOn(0, 38, 127);
      delay(100);
      noteOn(1, 38, 127);

      delay(1000);

      noteOff(0, 38, 127);
      delay(100);
      noteOff(1, 38, 127);
      delay(1000);

      break;
  }

  while (Serial.available()) {
    byte inChar = Serial.read();
    if (inChar == 'd') {
      state = DEBUG_MODE;
    } else if (inChar == 'n') {
      state = NORMAL;
    } else if (inChar == 'e') {
      state = EMULATE;
    } else if (inChar == 't') {
      pluckThresholds[0] -= 10;
    } else if (inChar == 'y') {
      pluckThresholds[0] += 10;
    }
  }
}
//-----------------------------------------------------------------------------
void updateSensorReadings() {
  for (int i = 0; i < numSensors; i++) {
    prevValues[i] = sensorValues[i];
    sensorValues[i] = analogRead(sensorPins[i]);
  }
}
//-----------------------------------------------------------------------------
void printReadings() {
  Serial.print(1024);
  Serial.print(' ');
  Serial.print(0);
  Serial.print(' ');
  for (int i = 0; i < numSensors; i++) {
    Serial.print(sensorValues[i]);
    Serial.print(' ');
  }
  for (int i = 0; i < numSensors; i++) {
    Serial.print(pluckThresholds[i]);
    Serial.print(' ');
  }
  Serial.println();
}
//-----------------------------------------------------------------------------
void writeRawDataToSerial() {
  for (int i = 0; i < numSensors; i++) {
    Serial.write((byte*)(sensorValues + i), 2);
  }
}

void datalog() {

  sensorLog1[datalogIndex] = analogRead(A0);
  sensorLog2[datalogIndex] = analogRead(A1);

  datalogIndex++;

  if (datalogIndex == dataLogSize) {
    datalogIndex = 0;
  }
}

//-----------------------------------------------------------------------------