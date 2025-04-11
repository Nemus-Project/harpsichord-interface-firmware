//-----------------------------------------------------------------------------
// Rotary Callbacks

void click(Button2& btn) {
  isKeySelectMode = !isKeySelectMode;
  if (isKeySelectMode) {
    r.setUpperBound(numSensors-1);
    r.setLowerBound(0);
    r.resetPosition(curKey, false);
  } else {
    r.setUpperBound(255);
    r.setLowerBound(0);
    r.resetPosition(pluckValues[curKey], false);
  }
}


void doubleclick(Button2& btn) {
  executeDebugMode = false;
}


void rotate(Rotary& r) {
  if (isKeySelectMode) {

    curKey = r.getPosition();

    for (auto pin : ledPins) {
      if (pin == ledPins[curKey]) {
        digitalWrite(pin, HIGH);
      } else {
        digitalWrite(pin, LOW);
      }
    }
  } else {
    pluckValues[curKey] = r.getPosition();
  }
}

