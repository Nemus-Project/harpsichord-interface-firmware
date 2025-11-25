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
  rotary.setChangedHandler(rotate);
  button.setDoubleClickHandler(doubleclick);
  button.setClickHandler(click);
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

  while (executeDebugMode) {

    // if (millis() - now > 16) {
    //  rainbow(step++);
    //   // breath(step++);
    //   now = millis();
    // }
    unsigned long start = micros();

    readSensors();
    rotary.loop();
    button.loop();

    for (int i = 0; i < numSensors; i++) {
      if (currSensorReadings[i] < pluckThresholds[i] and prevSensorReadings[i] > pluckThresholds[i]) {
        noteOff(0, index2note(i), 100);
      } else if (currSensorReadings[i] > pluckThresholds[i] and prevSensorReadings[i] < pluckThresholds[i]) {
        noteOn(0, index2note(i), 100);
      }
    }

    printJackReading(curKeyIndex);
    printJackThreshold(curKeyIndex);
    Serial.println();

    times[readCount] = micros() - start;
    readCount++;

    if (currSensorReadings[curKeyIndex] < sensorAvgMinima[curKeyIndex]) {
      sensorAvgMinima[curKeyIndex] = currSensorReadings[curKeyIndex];
    }

    if (readCount > 1024) {
      readCount = 0;

      // for (auto&& time : times) 
      for (int i = 0; i < 1024; i++)       
      {
        Serial.write((byte*)(times+i), 4);
        delay(1);
        // Serial.write((times+i), 4);
      }    
    }
  }

  exitDebug();
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
