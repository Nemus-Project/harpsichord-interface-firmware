//-----------------------------------------------------------------------------
// Rotary Callbacks

void click(Button2& btn) {
  isKeySelectMode = !isKeySelectMode;
  if (isKeySelectMode) {
    rotary.setIncrement(1);
    rotary.setUpperBound(numSensors);
    rotary.setLowerBound(0);
    rotary.resetPosition(curKeyIndex, false);
  } else {
    rotary.setIncrement(10);
    rotary.setUpperBound(1024);
    rotary.setLowerBound(0);
    rotary.resetPosition(pluckThresholds[curKeyIndex], false);
  }
}


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


void rotate(Rotary& r) {
  // Serial.println(r.directionToString(r.getDirection()));
  if (isKeySelectMode) {
    curKeyIndex = r.getPosition();
    leds.fill(leds.Color(0, 0, 0), 0, numSensors);
    leds.setPixelColor(curKeyIndex, 0, 0, 255);
    leds.show();
  } else {
    pluckThresholds[curKeyIndex] = r.getPosition();
  }
}
