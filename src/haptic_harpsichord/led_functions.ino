/**
 * @file led_functions.ino
 * @brief Utility functions for setting RGB LED behaviour
 *
 * Sets the behaviour of RGB LEDs on the sensor board
 *
 * @date 2025-10-11
 * @author Matthew Hamilton
 */

/**
 * @brief Set the Leds To Jack Displacement value from the sensor
 * 
 */
void setLedsToJackDisplacement() {
  for (int i = 0; i < numSensors; i++) {
    uint8_t c = map(currSensorReadings[i], 0, 1024, 255, 0);

    if (currSensorReadings[i] > singlePluckThresholds[i]) {
      leds.setPixelColor(i, c, 0, 0);
    } else {
      leds.setPixelColor(i, 0, c, 0);
    }
  }
  leds.show();
}

/**
 * @brief Rainbow effect using the NeoPixel library. Shoud be called through a timer function with an incrmeneting step value
 * 
 * @param step step withing the cycle
 */
void rainbow(int step) {
  leds.rainbow(step * 256);
  leds.show();  // Update strip with new contents
}

/**
 * @brief Breathing led effect. Shoud be called through a timer function with an incrmeneting step value
 * 
 * @param step index within the 8-bit 255 element long sine lookup table
 */
void breath(int step) {
  leds.setBrightness(leds.sine8(step));
  leds.fill(leds.Color(0xFF,0xFF,0XEE),0,numSensors);
  leds.show();
}
