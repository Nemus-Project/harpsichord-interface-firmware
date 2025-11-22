//-----------------------------------------------------------------------------
// Debug functions

void setupDebugMode() {
  digitalWrite(LEDB, LOW);
  digitalWrite(LEDB, LOW);
  executeDebugMode = true;
  rotary.setChangedHandler(rotate);
  button.setDoubleClickHandler(doubleclick);
  button.setClickHandler(click);
}


/// run loop for debug mode
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
  }

  exitDebug();
}

// void setCurrentKey() {
//   int thresh;
//   for (int i = 0; i < numSensors; i++) {
//     if (readSensor(i) < thresh and i != curKeyIndex) {
//       curKeyIndex = i;
//       leds.fill(leds.Color(0, 0, 0), 0, numSensors);
//       leds.setPixelColor(curKeyIndex, 0, 0, 100);
//       leds.show();
//     }
//   }
// }

void exitDebug() {
  digitalWrite(LEDB, LOW);
  digitalWrite(LEDB, LOW);
  writePluckToEEPROM();

  leds.clear();
}
