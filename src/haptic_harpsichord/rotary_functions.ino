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
  isKeySelectMode = !isKeySelectMode;
  if (isKeySelectMode) {
    rotary.setIncrement(1);
    rotary.setUpperBound(numSensors);
    rotary.setLowerBound(0);
    rotary.resetPosition(curKeyIndex, false);
  } else {
    rotary.setIncrement(10);
    // rotary.setUpperBound(1024);
    rotary.setUpperBound(4096);
    rotary.setLowerBound(0);
    rotary.resetPosition(singlePluckThresholds[curKeyIndex], false);
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
  // Serial.println(r.directionToString(r.getDirection()));
  if (isKeySelectMode) {
    curKeyIndex = r.getPosition();
    leds.fill(leds.Color(0, 0, 0), 0, numSensors);
    leds.setPixelColor(curKeyIndex, 0, 0, 255);
    leds.show();
  } else {
    singlePluckThresholds[curKeyIndex] = r.getPosition();
  }
}
