void printJackStates() {
  bool anyPrint = false;

  for (int i = 0; i < numSensors; i++) {
    if (jackStates[i] == PLUCK and prevStates[i] == RELEASED) {
      Serial.print("Jack ");
      Serial.print(i);
      Serial.print(" Plucked ");
      anyPrint = true;
      digitalWrite(ledPins[i], HIGH);
    }
  }

// OR
  // {
  //   int i = 5;

  //   if (jackStates[i] != prevStates[i]) {

  //     switch (jackStates[i]) {
  //       case PLUCKED:
  //         Serial.print("Jack 5 PLUCKED");
  //         break;
  //       case PLUCK:
  //         Serial.print("Jack 5 PLUCK");
  //         break;
  //       case PRESSING:
  //         Serial.print("Jack 5 PRESSING");
  //         break;
  //       case RELEASING:
  //         Serial.print("Jack 5 RELEASING");
  //         break;
  //       case RELEASED:
  //         Serial.print("Jack 5 RELEASED");
  //         break;
  //       case UNKNOWN_KEY_STATE:
  //         Serial.print("Jack 5 UNKNOWN_KEY_STATE");
  //         break;
  //     }      
  //     anyPrint = true;
  //   }
  // }

  if (anyPrint)
    Serial.println();
}

//-----------------------------------------------------------------------------
void printCurrentKeyAndThresh() {
  if (!isKeySelectMode)
    Serial.print("(edit)");
  Serial.print("p");
  Serial.print(curKey);
  Serial.print(":");
  Serial.print(pluckValues[curKey]);
  Serial.print('\t');

  Serial.print("s");
  Serial.print(curKey);
  Serial.print(":");
  Serial.print(sensorValues[curKey]);
  Serial.print('\t');

  Serial.print("min:");
  Serial.print(0);
  Serial.print('\t');
  Serial.print("max:");
  Serial.println(255);
}

void printReadings() {
  for (int i = 0; i < numSensors; i++) {
    Serial.print("s");
    Serial.print(i);
    Serial.print(":");
    Serial.print(sensorValues[i]);
    Serial.print('\t');
  }

  Serial.print("min:");
  Serial.print(0);
  Serial.print('\t');
  Serial.print("max:");
  Serial.println(255);
}