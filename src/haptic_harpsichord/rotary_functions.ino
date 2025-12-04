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
 * @brief Click calback
 * 
 * Switches between threshold change and key select modes
 *
 * @param btn reference to a Button2 library button object, which represents the momentary switch on the rotary encoder.
 */
void click(Button2& btn) {

  // Set next mode
  switch (rotaryMode) {
    case KEY_SELECT:
      rotaryMode = (thresholdType == SINGLE_THRESHOLD) ? EDIT_SINGLE_THRESHOLD : EDIT_PLUCK_THRESHOLD;
      break;
    case EDIT_PLUCK_THRESHOLD:
      rotaryMode = EDIT_RELEASE_THRESHOLD;
      break;
    case REGISTER_SELECT:
      waitingForInput = false;
    case EDIT_RELEASE_THRESHOLD:
    case EDIT_SINGLE_THRESHOLD:
      rotaryMode = KEY_SELECT;
      break;      
  }

  // New mode setup
  switch (rotaryMode) {
    case KEY_SELECT:
      rotary.setIncrement(1);
      rotary.setUpperBound(numSensors);
      rotary.setLowerBound(0);
      rotary.resetPosition(curKeyIndex, false);
      break;
    case EDIT_SINGLE_THRESHOLD:
      rotary.setIncrement(10);
      rotary.setUpperBound(4096);
      rotary.setLowerBound(0);
      rotary.resetPosition(singlePluckThresholds[curKeyIndex], false);
      break;
    case EDIT_PLUCK_THRESHOLD:
      rotary.setIncrement(10);
      rotary.setUpperBound(4096);
      rotary.setLowerBound(0);
      rotary.resetPosition(pluckThresholds[curKeyIndex], false);
      break;
    case EDIT_RELEASE_THRESHOLD:
      rotary.setIncrement(10);
      rotary.setUpperBound(4096);
      rotary.setLowerBound(0);
      rotary.resetPosition(releaseThresholds[curKeyIndex], false);
      break;
    case REGISTER_SELECT:
      rotary.setIncrement(1);
      rotary.setUpperBound(1);
      rotary.setLowerBound(0);
      rotary.resetPosition(0, false);
      break;    
  }
}

/**
 * @brief Double Click Callback
 *
 * Saves the current theshold values to EEPROM
 * 
 * @param btn reference to a Button2 library button object, which represents the momentary switch on the rotary encoder.
 */
void doubleclick(Button2& btn) {

  // executeDebugMode = false;
  // save to EEPROM
  writePluckToEEPROM();
  leds.fill(leds.Color(0, 0, 50), 0, numSensors);
  leds.show();
  delay(100);
  leds.fill(leds.Color(0, 0, 0), 0, numSensors);
  leds.setPixelColor(curKeyIndex, 0, 0, 100);
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

  switch (rotaryMode) {
    case KEY_SELECT:
      curKeyIndex = r.getPosition();
      leds.fill(leds.Color(0, 0, 0), 0, numSensors);
      leds.setPixelColor(curKeyIndex, 0, 0, 255);
      leds.show();
      break;
    case EDIT_SINGLE_THRESHOLD:
      singlePluckThresholds[curKeyIndex] = r.getPosition();
      break;
    case EDIT_PLUCK_THRESHOLD:
      pluckThresholds[curKeyIndex] = r.getPosition();
      break;
    case EDIT_RELEASE_THRESHOLD:
      releaseThresholds[curKeyIndex] = r.getPosition();
      break;
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

      leds.show();
      break;
  }
}
