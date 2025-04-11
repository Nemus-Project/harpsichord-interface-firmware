void blinkOnboardRGB(size_t ledPin, unsigned long duration, unsigned long pause = 0, byte count = 1) {

  for (int i = 0; i < count; i++) {
    digitalWrite(ledPin, LOW);
    delay(duration);
    digitalWrite(ledPin, HIGH);
    delay(duration);
  }

  delay(pause);
}

void setLedsToJackDisplacement() {
  for (int i = 0; i < numSensors; i++) {
    uint8_t c = map(currSensorReadings[i], 0, 1024, 255, 0);

    if (currSensorReadings[i] > pluckThresholds[i]) {
      leds.setPixelColor(i, c, 0, 0);
    } else {
      leds.setPixelColor(i, 0, c, 0);
    }
  }
  leds.show();
}

void rainbow(int step) {
  leds.rainbow(step * 256);
  leds.show();  // Update strip with new contents
}


void breath(int step) {
  leds.setBrightness(leds.sine8(step));
  leds.fill(leds.Color(0xFF,0xFF,0XEE),0,numSensors);
  leds.show();  // Update strip with new contents
}
