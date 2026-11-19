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
 * @deprecated DO NOT USE. Kept for reference only, see ThresholdType.
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
 * @brief Generate MIDI for all active keys with the selected detection method
 *
 * Shared by `hysteresisLoop()` and `hystereticDebugLoop()` so both play
 * identically. Call once after each `readSensors()`.
 *
 * @see DetectionMode, processHysteretic(), processVelocityDetection()
 */
void processNotes() {
  // Track the lowest velocity since the last Note Off (see minVelocitySinceRelease)
  for (int i = FIRST_ACTIVE_SENSOR; i <= LAST_ACTIVE_SENSOR; i++) {
    if (sensorVelocity[i] < minVelocitySinceRelease[i])
      minVelocitySinceRelease[i] = sensorVelocity[i];
  }

  if (detectionMode == VELOCITY_DETECTION)
    processVelocityDetection();
  else
    processHysteretic();
}

/**
 * @brief Generate MIDI from the velocity peak (no per-key calibration)
 *
 * Per key, while RELEASED:
 * 1. Arm when the velocity falls below -velocityOnThreshold (start of a
 *    press); remember the reading at that moment (`pressStartReading`).
 * 2. Track the lowest velocity since arming (`velocityPeak`).
 * 3. Note On once the velocity has risen back by VELOCITY_PEAK_DROP of that
 *    peak, i.e. just after the velocity peak.
 *
 * While PLUCK: track the lowest reading (`pressBottomReading`) and send Note
 * Off once the reading has come back up VELOCITY_OFF_FRACTION of the way from
 * that bottom to `pressStartReading`. Both levels come from the press itself,
 * so no thresholds need to be calibrated.
 *
 * The velocity lags the movement by about (VELOCITY_WINDOW - 1) / 2 scans,
 * and the peak is only recognised once the velocity rises again, so keep
 * VELOCITY_WINDOW short (see its comment).
 */
void processVelocityDetection() {
  for (int i = FIRST_ACTIVE_SENSOR; i <= LAST_ACTIVE_SENSOR; i++) {
    float v = sensorVelocity[i];
    int32_t reading = currSensorReadings[i];

    if (jackStates[i] == RELEASED) {
      if (not velocityArmed[i]) {
        if (v < -(float)velocityOnThreshold) {
          velocityArmed[i] = true;
          velocityPeak[i] = v;
          pressStartReading[i] = reading;
        }
      } else {
        if (v < velocityPeak[i])
          velocityPeak[i] = v;
        if (v > velocityPeak[i] * (1.0f - VELOCITY_PEAK_DROP)) {
          noteOn(0, index2note(i), 100);
          jackStates[i] = PLUCK;
          velocityArmed[i] = false;
          pressBottomReading[i] = reading;
        }
      }
    } else {
      if (reading < pressBottomReading[i])
        pressBottomReading[i] = reading;

      int32_t bottom = pressBottomReading[i];
      if (reading > bottom + VELOCITY_OFF_FRACTION * (float)(pressStartReading[i] - bottom)) {
        noteOff(0, index2note(i), 100);
        jackStates[i] = RELEASED;
        minVelocitySinceRelease[i] = 0.0f;
      }
    }
  }
}

/**
 * @brief Switch the note detection method at runtime
 *
 * Any note currently sounding is released first and every key's velocity
 * detection state is reset, so no note is left hanging and the new method
 * starts from a clean state.
 *
 * @param newMode detection method to switch to
 * @see DetectionMode
 */
void setDetectionMode(DetectionMode newMode) {
  for (int i = FIRST_ACTIVE_SENSOR; i <= LAST_ACTIVE_SENSOR; i++) {
    if (jackStates[i] == PLUCK) {
      noteOff(0, index2note(i), 100);
      jackStates[i] = RELEASED;
    }
    velocityArmed[i] = false;
    minVelocitySinceRelease[i] = 0.0f;
  }
  detectionMode = newMode;

  // Keep the encoder and LEDs in step if the mode was changed over serial (`d`)
  // while in DETECTION_SELECT
  if (rotaryMode == DETECTION_SELECT) {
    rotary.resetPosition(detectionMode == VELOCITY_DETECTION ? 1 : 0, false);
    showDetectionSelectLeds();
    leds.show();
  }

  Serial.print("Detection: ");
  Serial.println(detectionMode == VELOCITY_DETECTION ? "Velocity" : "Position");
}

/**
 * @brief Generate MIDI from hysteretic threshold crossings (position detection)
 *
 * Called by processNotes() when detectionMode is POSITION_DETECTION.
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
      minVelocitySinceRelease[i] = 0.0f;
    } else if (currSensorReadings[i] < pluckThresholds[i] and prevSensorReadings[i] >= pluckThresholds[i] and jackStates[i] == RELEASED) {
      noteOn(0, index2note(i), 100);
      jackStates[i] = PLUCK;
    }
  }
}
