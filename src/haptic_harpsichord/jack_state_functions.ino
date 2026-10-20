/**
 * @file jack_state_functions.ino
 * @brief Utilities for updating jack states and generating MIDI based on the change in state
 *
 * @date 2025-10-11
 * @author Matthew Hamilton
 */


/**
 * @brief Update the Jack state array given current sensor values
 * 
 */
void updateJackStates() {

  // tempStatePointer = prevStates;
  // prevStates = jackStates;
  // jackStates = tempStatePointer;

  for (int i = 0; i < numSensors; i++) {
    //   if (prevSensorReadings[i] <= releaseThresholds[i])
    //     jackStates[i] = RELEASED;
    //   else if (prevSensorReadings[i] < singlePluckThresholds[i] and currSensorReadings[i] > singlePluckThresholds[i])
    //     jackStates[i] = PLUCK;
    //   else if (prevSensorReadings[i] > singlePluckThresholds[i] and currSensorReadings[i] > singlePluckThresholds[i])
    //     jackStates[i] = PLUCKED;
    // }

    // if (prevSensorReadings[0] > pluckThresholds[0] and currSensorReadings[0] < pluckThresholds[0])
    //   jackStates[0] = RELEASING;
    // if (prevSensorReadings[1] > pluckThresholds[1] and currSensorReadings[1] < pluckThresholds[1])
    //   jackStates[1] = RELEASING;
    // if (prevSensorReadings[2] > pluckThresholds[2] and currSensorReadings[2] < pluckThresholds[2])
    //   jackStates[2] = RELEASING;
  }
}


/**
 * @brief Generate MIDI messages base on jack state
 * 
 */
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
//-----------------------------------------------------------------------------
/**
 * @brief Generate MIDI from single threshold crossings
 *
 * Shared by `singleThresholdLoop()` and `singleThresholdDebugLoop()` so both
 * play identically. Call once after each `readSensors()`.
 */
void processSingleThreshold() {
  for (int i = FIRST_ACTIVE_SENSOR; i <= LAST_ACTIVE_SENSOR; i++) {
    if (currSensorReadings[i] < singlePluckThresholds[i] and prevSensorReadings[i] > singlePluckThresholds[i]) {
      noteOn(0, index2note(i), 100);
    } else if (currSensorReadings[i] > singlePluckThresholds[i] and prevSensorReadings[i] < singlePluckThresholds[i]) {
      noteOff(0, index2note(i), 100);
    }
  }
}

/**
 * @brief Generate MIDI from hysteretic threshold crossings
 *
 * Shared by `hysteresisLoop()` and `hystereticDebugLoop()` so both play
 * identically. Call once after each `readSensors()`.
 *
 * A press makes the reading drop: Note On when it falls below the pluck
 * threshold, Note Off when it rises above the release threshold. The
 * `jackStates` gate guarantees on/off messages strictly alternate per key.
 */
void processHysteretic() {
  for (int i = FIRST_ACTIVE_SENSOR; i <= LAST_ACTIVE_SENSOR; i++) {
    if (currSensorReadings[i] > releaseThresholds[i] and prevSensorReadings[i] <= releaseThresholds[i] and jackStates[i] == PLUCK) {
      noteOff(0, index2note(i), 100);
      jackStates[i] = RELEASED;
    } else if (currSensorReadings[i] < pluckThresholds[i] and prevSensorReadings[i] >= pluckThresholds[i] and jackStates[i] == RELEASED) {
      noteOn(0, index2note(i), 100);
      jackStates[i] = PLUCK;
    }
  }
}
