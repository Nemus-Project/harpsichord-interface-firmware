//-----------------------------------------------------------------------------
// Debug functions

void setupDebugMode() {

  digitalWrite((FRONT_REGSISTER) ? LEDB : LEDG, LOW);
  executeDebugMode = true;
  rotary.setChangedHandler(rotate);
  button.setDoubleClickHandler(doubleclick);
  button.setClickHandler(click);
}


/// run loop for debug mode
void debugLoop() {
  setupDebugMode();

  while (executeDebugMode) {

    keyboardLoop();
    plotKeyData(curKeyIndex);
  }

  exitDebug();
}

void exitDebug() {
  digitalWrite(LEDB, LOW);
  writePluckToEEPROM();
  leds.clear();
}
