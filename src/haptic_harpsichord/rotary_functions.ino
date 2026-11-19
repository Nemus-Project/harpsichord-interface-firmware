/**
 * @file filename.ino
 * @brief Rotary Encoder Callbacks
 *
 * Any changes to rotary encoder behaviour should be implemented here.
 *
 * @date 2025-10-11
 * @author Matthew Hamilton
 */

/**
 * @brief Light the whole strip with the colour of the chosen detection mode
 *
 * Used in DETECTION_SELECT: green = position, orange = velocity.
 */
void showDetectionSelectLeds() {
  uint32_t color = (detectionMode == VELOCITY_DETECTION) ? velocityDetectionColor : positionDetectionColor;
  leds.fill(color, 0, numSensors);
}

/**
 * @brief Click calback
 *
 * Steps through the calibration modes. The cycle depends on the detection mode:
 *
 * - position: key select -> pluck threshold -> release threshold -> detection select
 * - velocity: key select -> velocity threshold -> detection select
 *
 * In detection select, turning switches between position and velocity
 * detection; the next click starts that mode's cycle at key select.
 *
 * @param btn reference to a Button2 library button object, which represents the momentary switch on the rotary encoder.
 */
void click(Button2& btn) {

  // Calibration controls are only active in debug mode
  if (!executeDebugMode)
    return;

  // Set next mode
  switch (rotaryMode) {
    case KEY_SELECT:
      if (thresholdType == SINGLE_THRESHOLD)
        rotaryMode = EDIT_SINGLE_THRESHOLD;
      else if (detectionMode == VELOCITY_DETECTION)
        rotaryMode = EDIT_VELOCITY_THRESHOLD;
      else
        rotaryMode = EDIT_PLUCK_THRESHOLD;
      break;
    case EDIT_PLUCK_THRESHOLD:
      rotaryMode = EDIT_RELEASE_THRESHOLD;
      break;
    case EDIT_RELEASE_THRESHOLD:
    case EDIT_VELOCITY_THRESHOLD:
      rotaryMode = DETECTION_SELECT;
      break;
    case REGISTER_SELECT:
      waitingForInput = false;
    case DETECTION_SELECT:
    case EDIT_SINGLE_THRESHOLD:
      rotaryMode = KEY_SELECT;
      break;
  }

  // New mode setup
  leds.fill(leds.Color(0, 0, 0), 0, numSensors);
  switch (rotaryMode) {
    case KEY_SELECT:
      rotary.setIncrement(INCREMENT);
      rotary.setUpperBound(MAX_POS);  // bounds are inclusive: MAX_POS = numSensors - 1
      rotary.setLowerBound(MIN_POS);
      rotary.resetPosition(curKeyIndex, false);
      leds.setPixelColor(curKeyIndex, keySelectColor);
      currentLedColor = keySelectColor;
      break;
    case EDIT_SINGLE_THRESHOLD:
      rotary.setIncrement(10);
      rotary.setUpperBound(SENSOR_READING_MAX);
      rotary.setLowerBound(0);
      rotary.resetPosition(singlePluckThresholds[curKeyIndex], false);
      leds.setPixelColor(curKeyIndex, editPluckThresholdColor);
      currentLedColor  = editPluckThresholdColor;
      break;
    case EDIT_PLUCK_THRESHOLD:
      rotary.setIncrement(10);
      rotary.setUpperBound(SENSOR_READING_MAX);
      rotary.setLowerBound(0);
      rotary.resetPosition(pluckThresholds[curKeyIndex], false);
      leds.setPixelColor(curKeyIndex, editPluckThresholdColor);
      currentLedColor  = editPluckThresholdColor;
      break;
    case EDIT_RELEASE_THRESHOLD:
      rotary.setIncrement(10);
      rotary.setUpperBound(SENSOR_READING_MAX);
      rotary.setLowerBound(0);
      rotary.resetPosition(releaseThresholds[curKeyIndex], false);
      leds.setPixelColor(curKeyIndex, editReleaseThresholdColor);
      currentLedColor  = editReleaseThresholdColor;
      break;
    case REGISTER_SELECT:
      rotary.setIncrement(1);
      rotary.setUpperBound(1);
      rotary.setLowerBound(0);
      rotary.resetPosition(0, false);
      break;
    case DETECTION_SELECT:
      rotary.setIncrement(1);
      rotary.setUpperBound(1);  // 0 = position, 1 = velocity
      rotary.setLowerBound(0);
      rotary.resetPosition(detectionMode == VELOCITY_DETECTION ? 1 : 0, false);
      showDetectionSelectLeds();
      break;
    case EDIT_VELOCITY_THRESHOLD:
      rotary.setIncrement(VELOCITY_ON_THRESHOLD_STEP);
      rotary.setUpperBound(VELOCITY_ON_THRESHOLD_MAX);
      rotary.setLowerBound(VELOCITY_ON_THRESHOLD_MIN);
      rotary.resetPosition(velocityOnThreshold, false);
      leds.setPixelColor(curKeyIndex, editVelocityThresholdColor);
      currentLedColor = editVelocityThresholdColor;
      break;
  }
  leds.show();
}

/**
 * @brief Double Click Callback
 *
 * Saves the current theshold values to EEPROM
 * 
 * @param btn reference to a Button2 library button object, which represents the momentary switch on the rotary encoder.
 */
void doubleclick(Button2& btn) {

  // Calibration controls are only active in debug mode
  if (!executeDebugMode)
    return;

  // save to EEPROM
  writePluckToEEPROM();
  leds.fill(leds.Color(0, 0, 50), 0, numSensors);
  leds.show();
  delay(100);
  leds.fill(leds.Color(0, 0, 0), 0, numSensors);
  if (rotaryMode == DETECTION_SELECT)
    showDetectionSelectLeds();
  else
    leds.setPixelColor(curKeyIndex, currentLedColor);
  leds.show();
}

/**
 * @brief Rotate callback
 * 
 * Effect of rotation based on value of `isKeySelectMode`. Either selects the key to calibrate or changes the threshold value.
 *
 * @param r reference to a Rotary library rotary object, which interfaces with the rotary encoder. 
 */
void rotate(Rotary& r) {

  leds.fill(leds.Color(0, 0, 0), 0, numSensors);

  switch (rotaryMode) {
    case KEY_SELECT:
      curKeyIndex = r.getPosition();
      leds.setPixelColor(curKeyIndex, keySelectColor);
      break;
    case EDIT_SINGLE_THRESHOLD:
      singlePluckThresholds[curKeyIndex] = r.getPosition();
      leds.setPixelColor(curKeyIndex, editPluckThresholdColor);
      break;
    case EDIT_PLUCK_THRESHOLD:
      pluckThresholds[curKeyIndex] = r.getPosition();
      leds.setPixelColor(curKeyIndex, editPluckThresholdColor);
      break;
    case EDIT_RELEASE_THRESHOLD:
      releaseThresholds[curKeyIndex] = r.getPosition();
      leds.setPixelColor(curKeyIndex, editReleaseThresholdColor);
      break;
    case EDIT_VELOCITY_THRESHOLD:
      velocityOnThreshold = r.getPosition();
      leds.setPixelColor(curKeyIndex, editVelocityThresholdColor);
      break;
    case DETECTION_SELECT: {
      DetectionMode chosen = r.getPosition() ? VELOCITY_DETECTION : POSITION_DETECTION;
      if (chosen != detectionMode)
        setDetectionMode(chosen);  // releases sounding notes, prints the mode
      showDetectionSelectLeds();
      break;
    }
    case REGISTER_SELECT:
      jackRegister = (r.getPosition()) ? FRONT_REGISTER : BACK_REGISTER;

      switch (jackRegister) {
        case FRONT_REGISTER:
          leds.fill(leds.Color(0, 0, 0), 0, numSensors);
          leds.fill(leds.Color(0, 100, 0), numSensors / 2, numSensors / 2);
          break;
        case BACK_REGISTER:
          leds.fill(leds.Color(0, 0, 0), 0, numSensors);
          leds.fill(leds.Color(0, 100, 0), 0, numSensors / 2);
          break;
      }

      break;
  }

  leds.show();
}

/**
 * @brief Long Click Callback
 *
 * Toggles between play mode and debug (calibration) mode. Fires as soon as the
 * button has been held for `MODE_SWITCH_HOLD_MS`; the running loop then returns
 * and `loop()` enters the other mode. Leaving debug mode saves thresholds.
 *
 * @param btn reference to a Button2 library button object, which represents the momentary switch on the rotary encoder.
 */
void longClickDetected(Button2& btn) {
  executeDebugMode = !executeDebugMode;
}
