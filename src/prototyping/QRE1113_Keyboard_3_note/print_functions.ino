void printJackStates() {
  bool anyPrint = false;

  if (jackStates[0] == PLUCK and prevStates[0] == RELEASED) {
    Serial.print("Jack 1 Plucked ");
    anyPrint = true;
  }
  if (jackStates[1] == PLUCK and prevStates[1] == RELEASED) {
    Serial.print("Jack 2 Plucked ");
    anyPrint = true;
  }
  if (jackStates[2] == PLUCK and prevStates[2] == RELEASED) {
    Serial.print("Jack 3 Plucked ");
    anyPrint = true;
  }
  if (anyPrint)
    Serial.println();
}

//-----------------------------------------------------------------------------

void printReadings() {
  Serial.print("min:");
  Serial.print(0);
  Serial.print(' ');

  Serial.print("max:");
  Serial.print(255);
  Serial.print(' ');

  if (!isKeySelectMode and curKey == 0)
    Serial.print("(edit)");
  Serial.print("p1:");
  Serial.print(pluckValues[0]);
  Serial.print(' ');

  if (!isKeySelectMode and curKey == 1)
    Serial.print("(edit)");
  Serial.print("p2:");
  Serial.print(pluckValues[1]);
  Serial.print(' ');

  if (!isKeySelectMode and curKey == 2)
    Serial.print("(edit)");
  Serial.print("p3:");
  Serial.print(pluckValues[2]);
  Serial.print(' ');

  Serial.print("s1:");
  Serial.print(sensorValues[0]);
  Serial.print(' ');

  Serial.print("s2:");
  Serial.print(sensorValues[1]);
  Serial.print(' ');

  Serial.print("s3:");
  Serial.println(sensorValues[2]);
}