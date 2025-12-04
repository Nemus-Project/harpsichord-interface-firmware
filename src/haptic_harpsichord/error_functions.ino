void blinkRGB(size_t ledPin, unsigned long duration, unsigned long pause = 0, byte count = 1) {

  for (int i = 0; i < count; i++) {
    digitalWrite(ledPin, LOW);
    delay(duration);
    digitalWrite(ledPin, HIGH);
    delay(duration);
  }

  delay(pause);
}

void halt(HapticHarpsichordErrorCodes errorCode) {

  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  while (true) {
        // blinkRGB(LEDB, 500, 2000, 3);
        for (int i = 0; i < 3; i++) {          
          digitalWrite(LED_BUILTIN, HIGH);          
          delay(500);
          digitalWrite(LED_BUILTIN, LOW);
          delay(500);
          // leds.fill(leds.Color(20, 0, 0), 0, numSensors);
          // leds.fill(leds.Color(0, 0, 0), 0, numSensors);
        }
        delay(3000);
      }

  // switch (errorCode) {
  //   case FRAM_NOT_FOUND:
  //     while (true) {
  //       // blinkRGB(LEDB, 500, 2000, 3);
  //       for (int i = 0; i < 3; i++) {          
  //         digitalWrite(LED_BUILTIN, HIGH);          
  //         delay(500);
  //         digitalWrite(LED_BUILTIN, LOW);
  //         delay(500);
  //         // leds.fill(leds.Color(20, 0, 0), 0, numSensors);
  //         // leds.fill(leds.Color(0, 0, 0), 0, numSensors);
  //       }
  //       delay(3000);
  //     }
  //     break;
  //   default:
  //     break;
  // }
}
