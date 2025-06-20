//-----------------------------------------------------------------------------
// Rotary Callbacks

void click(Button2& btn) {

  switch (currenRotaryMode) {
    case KEY_SELECT:
      currenRotaryMode = EDIT_PLUCK_THRESHOLD;
      break;
    case EDIT_PLUCK_THRESHOLD:
      currenRotaryMode = EDIT_RELEASE_THRESHOLD;
      break;
    case EDIT_RELEASE_THRESHOLD:
      currenRotaryMode = KEY_SELECT;
      break;
  }

  switch (currenRotaryMode) {
    case KEY_SELECT:
      currentLedColor = keySelectColor;
      rotary.setIncrement(1);
      rotary.setUpperBound(numSensors);
      rotary.setLowerBound(0);
      rotary.resetPosition(curKeyIndex, false);
      break;
    case EDIT_PLUCK_THRESHOLD:
      currentLedColor = editPluckThresholdColor;
      rotary.setIncrement(10);
      rotary.setUpperBound(1024);
      rotary.setLowerBound(0);
      rotary.resetPosition(pluckThresholds[curKeyIndex], false);
      break;
    case EDIT_RELEASE_THRESHOLD:
      currentLedColor = editReleaseThresholdColor;
      rotary.setIncrement(10);
      rotary.setUpperBound(1024);
      rotary.setLowerBound(0);
      rotary.resetPosition(releaseThresholds[curKeyIndex], false);
      break;
  }

  leds.setPixelColor(curKeyIndex, currentLedColor);
  leds.show();
}


void doubleclick(Button2& btn) {

  // save to EEPROM
  writePluckToEEPROM();
  leds.fill(leds.Color(0, 0, 50), 0, numSensors);
  leds.show();
  delay(100);
  leds.fill(leds.Color(0, 0, 0), 0, numSensors);
  leds.setPixelColor(curKeyIndex, 0, 0, 100);
  leds.show();
}


void rotate(Rotary& r) {

  switch (currenRotaryMode) {
    case KEY_SELECT:
      curKeyIndex = r.getPosition();
      leds.fill(leds.Color(0, 0, 0), 0, numSensors);
      leds.setPixelColor(curKeyIndex, currentLedColor);
      leds.show();
      break;
    case EDIT_PLUCK_THRESHOLD:
      pluckThresholds[curKeyIndex] = r.getPosition();
      break;
    case EDIT_RELEASE_THRESHOLD:
      releaseThresholds[curKeyIndex] = r.getPosition();
      break;
  }

}
