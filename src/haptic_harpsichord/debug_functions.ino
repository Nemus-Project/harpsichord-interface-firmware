/**
 * @file debug_functions.ino
 * @brief Functions for setting, running and exiting the firmware's "debug mode"
 *
 * `debugLoop()` is a substitute of the main loop. `setupDebugMode()` and
 * `exitDebug()` deal with setup and tear down of any changes to hardware behaviour.
 * 
 * NOTEL the intended behavour is for calibration values to be saved on exit. 
 * If values are edited and they should not be saves, press the reset button.
 *
 * @date 2025-10-11
 * @author Matthew Hamilton
 */

/**
 * @brief Setup procedure before starying the debug loop 
 *
 * Sets the `executeDebugMode` flag and the onboard RGB LED of the Nano BLE as omode indicator.
 * * Rotary encode callbacks are also set to allow for calibration.
 */
void setupDebugMode() {
  digitalWrite(LEDB, LOW);
  digitalWrite(LEDB, LOW);
  executeDebugMode = true;
}

/**
 * @brief Debug mode programme loop
 * 
 * Identical to the noraml `loop` except for the added listening for rotary callbacks
 * and Serial printing which adds to latency. 
 */
void debugLoop() {
  setupDebugMode();

  // curKeyIndex = rotary.getPosition();
  leds.fill(leds.Color(0, 0, 0), 0, numSensors);
  leds.setPixelColor(curKeyIndex, 0, 0, 255);
  leds.show();

  switch (thresholdType) {
    case SINGLE_THRESHOLD:
      singleThresholdDebugLoop();
      break;
    case HYSTERETIC:
      hystereticDebugLoop();
      break;
  }

  exitDebug();
}

void singleThresholdDebugLoop() {
  while (executeDebugMode) {

    readSensors();
    rotary.loop();
    button.loop();

    for (int i = 0; i < numSensors; i++) {
      if (currSensorReadings[i] < singlePluckThresholds[i] and prevSensorReadings[i] > singlePluckThresholds[i]) {
        noteOn(0, index2note(i), 100);
      } else if (currSensorReadings[i] > singlePluckThresholds[i] and prevSensorReadings[i] < singlePluckThresholds[i]) {
        noteOff(0, index2note(i), 100);
      }
    }
    // if (Serial.available() && Serial.read() == 'p')
    //   shouldPrint = !shouldPrint;

    if (shouldPrint) {
      printJackReading(curKeyIndex, 0, 4098);
      printJackThreshold(curKeyIndex);
      Serial.println();
    }
  }
}

void hystereticDebugLoop() {
  while (executeDebugMode) {

    readSensors();
    rotary.loop();
    button.loop();

    for (int i = 0; i < numSensors; i++) {
      if (currSensorReadings[i] > releaseThresholds[i] and prevSensorReadings[i] <= releaseThresholds[i] and jackStates[i] == PLUCK) {
        noteOff(0, index2note(i), 100);
        jackStates[i] = RELEASED;
      } else if (currSensorReadings[i] < pluckThresholds[i] and prevSensorReadings[i] >= pluckThresholds[i] and jackStates[i] == RELEASED) {
        noteOn(0, index2note(i), 100);
        jackStates[i] = PLUCK;
      }
    }

    if (Serial.available() && Serial.read() == 'p')
      shouldPrint = !shouldPrint;

    if (shouldPrint) {
      printJackReading(curKeyIndex, 1100, 1300);
      printJackThreshold(curKeyIndex);
      Serial.println();
    }
  }
}


/**
 * @brief 
 * 
 * Turn off LEDs and if any changes have been made they are written to NV RAM
 */
void exitDebug() {
  digitalWrite(LEDB, LOW);
  digitalWrite(LEDB, LOW);
  writePluckToEEPROM();

  leds.clear();
}
