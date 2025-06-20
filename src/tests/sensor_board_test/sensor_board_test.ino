// Single Jack Test
const byte numSensors = 7;
pin_size_t sensorPins[numSensors] = { A0, A1, A2, A3, A4, A5, A6 };
// pin_size_t sensorPins[numSensors] = {A0};
unsigned long sensorHomeValues[numSensors];
int sensorValues[numSensors];

// Multiplexer Variables
const size_t muxPinA = 6;
const size_t muxPinB = 7;
const size_t muxPinC = 8;
// const size_t muxPinD = 9;

void setup() {
  Serial.begin(9600);
  // calibrate();

  pinMode(muxPinA, OUTPUT);
  pinMode(muxPinB, OUTPUT);
  pinMode(muxPinC, OUTPUT);
  // switch mux channel
  uint8_t mux = 1;

  // digitalWrite(muxPinA, 1);
  // digitalWrite(muxPinB, (mux >> 1) & 0x1);
  // digitalWrite(muxPinC, (mux >> 2) & 0x1);

  digitalWrite(muxPinA, (mux >> 0) & 0x1);
  digitalWrite(muxPinB, (mux >> 1) & 0x1);
  digitalWrite(muxPinC, (mux >> 2) & 0x1);
}

void loop() {
  // updateSensorReadings();
  printValues();
}

void printValues() {
  for (int i = 0; i < numSensors; i++) {
    Serial.print(i);
    Serial.print(':');
    Serial.print(analogRead(sensorPins[i]));
    Serial.print(' ');
  }
  Serial.print("Min:");
  Serial.print(0);
  Serial.print(' ');
  Serial.print("Max:");
  Serial.println(1024);
}

void calibrate() {
  Serial.print("Calibrating.... ");

  for (int j = 0; j < 8; j++)
    for (int i = 0; i < numSensors; i++)
      sensorHomeValues[i] += analogRead(sensorPins[i]);

  for (int i = 0; i < numSensors; i++)
    sensorHomeValues[i] >>= 3;

  Serial.print("Done!");
}

void updateSensorReadings() {
  for (int i = 0; i < numSensors; i++) {
    sensorValues[i] = map(analogRead(sensorPins[i]), sensorHomeValues[i], 0, 0, 255);
    sensorValues[i] = (sensorValues[i] < 0) ? 0 : sensorValues[i];
  }
}
