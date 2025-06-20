

void updateJackStates() {

  tempStatePointer = prevStates;
  prevStates = jackStates;
  jackStates = tempStatePointer;

  for (int i = 0; i < numSensors; i++) {
    if (prevSensorReadings[i] <= releaseThresholds[i])
      jackStates[i] = RELEASED;
    else if (prevSensorReadings[i] < pluckThresholds[i] and currSensorReadings[i] > pluckThresholds[i])
      jackStates[i] = PLUCK;
    else if (prevSensorReadings[i] > pluckThresholds[i] and currSensorReadings[i] > pluckThresholds[i])
      jackStates[i] = PLUCKED;
  }
}

void checkJackStates() {

  bool anyTrigger = false;
  for (int i = 0; i < numSensors; i++) {
    if (jackStates[i] == PLUCK and prevStates[i] == RELEASED) {
      byte velocity = getOnVelocity(i);
      noteOn(0, index2note(i), velocity);
    } else if (jackStates[i] == RELEASED and prevStates[i] != RELEASED) {
      byte velocity = abs(prevSensorReadings[i] - currSensorReadings[i]);      
      noteOff(0, index2note(i), velocity);    
    }
  }

  if (anyTrigger)
    ;
}