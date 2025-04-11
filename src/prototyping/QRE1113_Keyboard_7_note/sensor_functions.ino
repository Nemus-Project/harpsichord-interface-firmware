//-----------------------------------------------------------------------------

void calibrate() {
  Serial.print("Calibrating.... ");

  for (int j = 0; j < 8; j++)
    for (int i = 0; i < numSensors; i++)
      sensorHomeValues[i] += analogRead(sensorPins[i]);

  for (int i = 0; i < numSensors; i++)
    sensorHomeValues[i] >>= 3;

  Serial.print("Done!");
}

//-----------------------------------------------------------------------------

void updateSensorReadings() {
  for (int i = 0; i < numSensors; i++) {
    prevValues[i] = sensorValues[i];
    sensorValues[i] = map(analogRead(sensorPins[i]), sensorHomeValues[i], 130, 0, 255);
    sensorValues[i] = (sensorValues[i] < 0) ? 0 : sensorValues[i];
  }
}

//-----------------------------------------------------------------------------

void updateJackStates() {

  for (int i = 0; i < numSensors; i++) {
    prevStates[i] = jackStates[i];
    if (prevValues[i] <= releaseValues[i]) {
      jackStates[i] = RELEASED;
    } else if (prevValues[i] < pluckValues[i] and sensorValues[i] > pluckValues[i]) {
      jackStates[i] = PLUCK;
    } else if (prevValues[i] > pluckValues[i] and sensorValues[i] > pluckValues[i]) {
      jackStates[i] = PLUCKED;
    }
  }
}

//-----------------------------------------------------------------------------

void checkJackStates() {
  bool anyTrigger = false;

  for (int i = 0; i < numSensors; i++) {
    if (jackStates[i] == PLUCK and prevStates[i] == RELEASED) {
      byte velocity = getVelocity(i);
      Serial.print("velocity: ");
      Serial.println(velocity);
      noteOn(0, noteMap[i], velocity);
      if (ledFeedback) {
        digitalWrite(ledPins[i], HIGH);
      }

    } else if (jackStates[i] == RELEASED and prevStates[i] != RELEASED) {
      byte velocity = prevValues[i] - sensorValues[i];
      noteOff(0, noteMap[i], velocity);
      if (ledFeedback) {
        digitalWrite(ledPins[i], LOW);
      }
    }
  }
}


byte getOnVelocity(int j) {
  byte velocity;
  if (j < 3) {

    velocity = map(sensorValues[j] - prevValues[j], 1, 180, 1, 63);
    if (velocity < 1)
      velocity = 1;
    else if (velocity > 63)
      velocity = 63;

  } else {
    {
      velocity = map(sensorValues[j] - prevValues[j], 1, 180, 65, 127);
      if (velocity < 65)
        velocity = 65;
      else if (velocity > 127)
        velocity = 127;
    }
  }
}

byte getOffVelocity(int j) {
  byte velocity;
  if (j < 3) {

    velocity = map(sensorValues[j] - prevValues[j], 1, 180, 1, 63);
    if (velocity < 1)
      velocity = 1;
    else if (velocity > 63)
      velocity = 63;

  } else {
    {
      velocity = map(sensorValues[j] - prevValues[j], 1, 180, 65, 127);
      if (velocity < 65)
        velocity = 65;
      else if (velocity > 127)
        velocity = 127;
    }
  }
}
