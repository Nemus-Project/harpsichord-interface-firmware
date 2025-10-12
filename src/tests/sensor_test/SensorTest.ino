/**
 * @file SensorTest.ino 
 * @brief Test a single sensor on each board
 * 
 */

const byte numSensors = 6;
pin_size_t sensorPins[numSensors] = { A0, A1, A2, A3, A6, A7 };
unsigned long sensorHomeValues[numSensors];
int sensorValues[numSensors];


void setup() {
  Serial.begin(9600);
  calibrate();
}

void loop() {
  updateSensorReadings();
  printValues();
}

void printValues() {
  for (int i = 0; i < 6; i++) {
    Serial.print(i);
    Serial.print(':');
    Serial.print(sensorValues[i]);
    Serial.print(' ');
  }
  Serial.print("Min:");
  Serial.print(0);
  Serial.print(' ');
  Serial.print("Max:");
  Serial.println(255);
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
