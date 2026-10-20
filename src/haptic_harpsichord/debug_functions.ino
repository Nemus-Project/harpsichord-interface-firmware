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

    processSingleThreshold();
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

    processHysteretic();

   if (Serial.available()) {
     char c = Serial.read();
     if (c == 'p') shouldPrint = !shouldPrint;
   else if (c == 'c') autoCalibrate();
  }

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
  digitalWrite(LEDB, HIGH);  // onboard RGB LED is active low: HIGH turns it off
  writePluckToEEPROM();

  // Return the encoder to key select so re-entering debug mode starts clean
  rotaryMode = KEY_SELECT;
  rotary.setIncrement(INCREMENT);
  rotary.setUpperBound(MAX_POS);
  rotary.setLowerBound(MIN_POS);
  rotary.resetPosition(curKeyIndex, false);
  currentLedColor = keySelectColor;

  leds.clear();
  leds.show();
}
