/**
 * @file error_functions.ino
 * @brief Utility functions in case of error
 *
 * Utility function to either halt the firmware or feedback the correct error code
 * using the onboard LEDs 
 *
 * @date 2025-10-11
 * @author Matthew Hamilton
 */

/**
 * @brief 
 * 
 * @param ledPin 
 * @param duration 
 * @param pause 
 * @param count 
 */
void blinkRGB(size_t ledPin, unsigned long duration, unsigned long pause = 0, byte count = 1) {

  for (int i = 0; i < count; i++) {
    digitalWrite(ledPin, LOW);
    delay(duration);
    digitalWrite(ledPin, HIGH);
    delay(duration);
  }

  delay(pause);
}

/**
 * @brief 
 * 
 * @param errorCode 
 */
void halt(HapticHarpsichordErrorCodes errorCode) {

  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  switch (errorCode) {
    case FRAM_NOT_FOUND:
      while (true) {
        // blinkRGB(LEDB, 500, 2000, 3);
        for (int i = 0; i < 3; i++) {
          leds.fill(leds.Color(20, 0, 0), 0, numSensors);
          delay(500);
          leds.fill(leds.Color(0, 0, 0), 0, numSensors);
          delay(500);
        }
        delay(3000);
      }
      break;
    default:
      break;
  }
}
