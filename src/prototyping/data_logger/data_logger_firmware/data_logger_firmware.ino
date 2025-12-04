/**
 * @file data_logger_firmware.ino
 * @brief Data logging sensors connected to ADC
 * 
 * Use the accompanying processing sketch to save binary data transmitted over serial
 * to a file.
 * 
 * Central Loop is a state machine to allow switching between monitoring data via 
 * Arduino IDE's serial plotter and steppinhg through recording and transmission.
 * 
 * Waits for Pin 12 to go high before continuing and will blink the LED_BUILTIN
 * when finished.
 * 
 * Serial messages:
 * 
 * - `w`: sets to the waiting state
 * - `d`: set to pretty print data for serial plotter
 * 
*/
//-----------------------------------------------------------------------------
#define ADC_RES 12
//-----------------------------------------------------------------------------
enum OPMODE { WAITING,
              RECORDING,
              SENDING,
              FINISHED,
              DEBUG_MODE,
};

OPMODE state = WAITING;
//-----------------------------------------------------------------------------
const pin_size_t sensorPins[] = {A0,A1,A2,A3};
const float VCC = 3.23;
const float R_DIV = 45800.0f;
bool mode = true;

#if ADC_RES == 12
const uint16_t lookupSize = 4096;
#else
const uint16_t lookupSize = 1024;
#endif

const size_t logSize = 25000; // Largest available buffer for remainder of BLE Nano memory.
uint16_t dataLog[4][logSize];
size_t logIndex = 0;

pin_size_t triggerPin = 12;
//-----------------------------------------------------------------------------
void setup() {
  Serial.begin(230400);

  for (auto pin : sensorPins) {
    pinMode(pin, INPUT);
  }


  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(triggerPin, INPUT_PULLUP);

  analogReadResolution(ADC_RES);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {

  switch (state) {
    case WAITING:

      if (digitalRead(triggerPin) == HIGH) {
        digitalWrite(LED_BUILTIN, LOW);
        state = RECORDING;
        logIndex = 0;
      } else {
        digitalWrite(LED_BUILTIN, HIGH);
      }
      break;
    case RECORDING:
      if (!logData()) {
        digitalWrite(LED_BUILTIN, HIGH);
        state = SENDING;
      }
      break;
    case SENDING:
      sendRawDataLog();
      state = FINISHED;
      break;
    case FINISHED:
      digitalWrite(LED_BUILTIN, (millis() / 1000) % 2);
      break;
    case DEBUG_MODE:
      printRaw();
      break;
  }


  while (Serial.available()) {
    byte inChar = Serial.read();
    if (inChar == 'd') {
      state = DEBUG_MODE;
    } else if (inChar == 'w') {
      state = WAITING;
    }
  }
}
//-----------------------------------------------------------------------------
void sendRawDataLog() {
  for (int i = 0; i < logSize; i++) {
    for (int j = 0; j < 4; j++)
      Serial.write((byte*)(dataLog[j] + i), 2);
  }
}

void printRaw() {
  Serial.print("M:");
  Serial.print(lookupSize);
  Serial.print(' ');
  Serial.print("m:");
  Serial.print(0);
  Serial.print(' ');

  static char buffer[4];

  for (auto pin : sensorPins) {
    Serial.print(itoa(pin, buffer, 10));
    Serial.print(":");
    Serial.print(analogRead(pin));
    Serial.print(' ');
  }

  Serial.println();
}
//-----------------------------------------------------------------------------
// void printWeight() {
//   int fsrADC[] = { analogRead(sensorPins[0]), analogRead(sensorPins[0]) };

//   Serial.print("M:");
//   Serial.print(100);
//   Serial.print(' ');
//   Serial.print("m:");
//   Serial.print(0);
//   Serial.print(' ');
//   Serial.print("Weight(g):");
//   Serial.print(weightLookup[fsrADC[0]]);
//   Serial.print(' ');
//   Serial.print(weightLookup[fsrADC[1]]);
//   Serial.print(' ');
//   Serial.println();
// }
//-----------------------------------------------------------------------------

// void printResistance() {
//   float fsrADC = float(analogRead(sensorPins[0]));
//   // float fsrV = fsrADC * VCC / 1023.0;
//   float fsrV = fsrADC * VCC / 4095.0f;
//   float fsrR = R_DIV * (VCC / fsrV - 1.0f);
//   Serial.println("Resistance: " + String(fsrR) + " ohms");
//   delay(1000);
// }

bool logData() {

  for (int i = 0; i < 4; i++)
    dataLog[i][logIndex] = analogRead(sensorPins[i]);

  logIndex++;

  if (logIndex == logSize) {
    return false;
  }
  return true;
}