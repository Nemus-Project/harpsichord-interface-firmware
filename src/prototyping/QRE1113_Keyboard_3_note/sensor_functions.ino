//-----------------------------------------------------------------------------

void calibrate() {
  Serial.print("Calibrating.... ");

  for (int j = 0; j < 8; j++)
    for (int i = 0; i < 3; i++)
      sensorHomeValues[i] += analogRead(sensorPins[i]);

  for (int i = 0; i < 3; i++)
    sensorHomeValues[i] >>= 3;

  Serial.print("Done!");
}

//-----------------------------------------------------------------------------

void updateSensorReadings() {
  prevValues[0] = sensorValues[0];
  prevValues[1] = sensorValues[1];
  prevValues[2] = sensorValues[2];
  sensorValues[0] = map(analogRead(sensorPins[0]), sensorHomeValues[0], 130, 0, 255);
  sensorValues[1] = map(analogRead(sensorPins[1]), sensorHomeValues[1], 130, 0, 255);
  sensorValues[2] = map(analogRead(sensorPins[2]), sensorHomeValues[2], 130, 0, 255);
  sensorValues[0] = (sensorValues[0] < 0) ? 0 : sensorValues[0];
  sensorValues[1] = (sensorValues[1] < 0) ? 0 : sensorValues[1];
  sensorValues[2] = (sensorValues[2] < 0) ? 0 : sensorValues[2];
  // sensorValues[0] = analogRead(sensorPins[0]);
  // sensorValues[1] = analogRead(sensorPins[1]);
  // sensorValues[2] = analogRead(sensorPins[2]);
}

//-----------------------------------------------------------------------------

void updateJackStates() {

  prevStates[0] = jackStates[0];
  prevStates[1] = jackStates[1];
  prevStates[2] = jackStates[2];


  if (prevValues[0] <= releaseValues[0])
    jackStates[0] = RELEASED;
  if (prevValues[1] <= releaseValues[1])
    jackStates[1] = RELEASED;
  if (prevValues[2] <= releaseValues[2])
    jackStates[2] = RELEASED;

  if (prevValues[0] < pluckValues[0] and sensorValues[0] > pluckValues[0])
    jackStates[0] = PLUCK;
  if (prevValues[1] < pluckValues[1] and sensorValues[1] > pluckValues[1])
    jackStates[1] = PLUCK;
  if (prevValues[2] < pluckValues[2] and sensorValues[2] > pluckValues[2])
    jackStates[2] = PLUCK;

  if (prevValues[0] > pluckValues[0] and sensorValues[0] > pluckValues[0])
    jackStates[0] = PLUCKED;
  if (prevValues[1] > pluckValues[1] and sensorValues[1] > pluckValues[1])
    jackStates[1] = PLUCKED;
  if (prevValues[2] > pluckValues[2] and sensorValues[2] > pluckValues[2])
    jackStates[2] = PLUCKED;

  // if (prevValues[0] > pluckValues[0] and sensorValues[0] < pluckValues[0])
  //   jackStates[0] = RELEASING;
  // if (prevValues[1] > pluckValues[1] and sensorValues[1] < pluckValues[1])
  //   jackStates[1] = RELEASING;
  // if (prevValues[2] > pluckValues[2] and sensorValues[2] < pluckValues[2])
  //   jackStates[2] = RELEASING;
}




//-----------------------------------------------------------------------------

void checkJackStates() {
  bool anyTrigger = false;
  if (jackStates[0] == PLUCK and prevStates[0] == RELEASED) {
    noteOn(0, 70, 127);
    anyTrigger = true;
  }
  if (jackStates[1] == PLUCK and prevStates[1] == RELEASED) {
    noteOn(0, 67, 127);
    anyTrigger = true;
  }
  if (jackStates[2] == PLUCK and prevStates[2] == RELEASED) {
    noteOn(0, 64, 127);
    anyTrigger = true;
  }
  if (jackStates[0] == RELEASED and prevStates[0] != RELEASED) {
    noteOff(0, 70, 127);
  }
  if (jackStates[1] == RELEASED and prevStates[1] != RELEASED) {
    noteOff(0, 67, 127);
  }
  if (jackStates[2] == RELEASED and prevStates[2] != RELEASED) {
    noteOff(0, 64, 127);
  }


  if (anyTrigger)
    ;
}

byte getVelocity() {
  // value before
  // value after
  // millis in between
  millis();
  return 0;
}
