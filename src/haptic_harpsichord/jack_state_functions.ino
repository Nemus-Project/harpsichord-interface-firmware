

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

  // if (prevSensorReadings[0] > pluckThresholds[0] and currSensorReadings[0] < pluckThresholds[0])
  //   jackStates[0] = RELEASING;
  // if (prevSensorReadings[1] > pluckThresholds[1] and currSensorReadings[1] < pluckThresholds[1])
  //   jackStates[1] = RELEASING;
  // if (prevSensorReadings[2] > pluckThresholds[2] and currSensorReadings[2] < pluckThresholds[2])
  //   jackStates[2] = RELEASING;
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