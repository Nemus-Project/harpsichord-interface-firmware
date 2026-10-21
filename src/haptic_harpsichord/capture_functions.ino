/**
 * @file capture_functions.ino
 * @brief Record one key press scan by scan, for offline analysis
 *
 * Debug mode only. Serial command `r` arms a capture of the selected key.
 * While armed, every scan is written to a ring buffer. The capture triggers
 * when the key's reading falls below its release threshold. It keeps up to
 * CAPTURE_PRE scans from before the trigger (rest and start of the free
 * travel), and ends once the reading has stayed above the release threshold
 * for CAPTURE_POST scans (key released), or when the buffer is full.
 *
 * Nothing is printed while armed or recording, so the scan timing is the same
 * as in normal use. The capture is then dumped over serial as CSV between
 * `[CAPTURE] BEGIN` and `[CAPTURE] END` lines (see tools/press_capture.py):
 *
 * - `t_us`: scan start time, microseconds from the first sample
 * - `raw`: raw 12-bit sample
 * - `reading`: filtered 13-bit reading, as used for note detection
 * - `note_on`: 1 while the key's note is on
 *
 * Raw samples are recorded so that velocity can be recomputed offline with
 * any method or window length.
 *
 * @date 2026-10-21
 */

/**
 * @brief Arm a capture of the selected key, or cancel the running one
 *
 * Pauses plot printing while capturing and lights the captured key white.
 */
void toggleCapture() {
  if (captureState != CAPTURE_IDLE) {
    endCapture();
    Serial.println("[CAPTURE] cancelled");
    return;
  }

  captureKey = curKeyIndex;
  captureHead = 0;
  captureFilled = 0;
  captureLength = 0;
  capturePreKept = 0;
  captureAboveCount = 0;

  captureSavedPrint = shouldPrint;
  shouldPrint = false;

  Serial.print("[CAPTURE] armed: key ");
  Serial.print(index2key(captureKey));
  Serial.print(", trigger when reading falls below release threshold ");
  Serial.print(releaseThresholds[captureKey]);
  Serial.println(". Press the key. Send r again to cancel.");

  leds.fill(leds.Color(0, 0, 0), 0, numSensors);
  leds.setPixelColor(captureKey, leds.Color(30, 30, 30));
  leds.show();

  captureState = CAPTURE_ARMED;
}

/**
 * @brief Record the current scan into the capture buffer
 *
 * Call once per scan, after readSensors() and processHysteretic(). Does
 * nothing unless a capture is armed or recording.
 */
void updateCapture() {
  if (captureState == CAPTURE_IDLE)
    return;

  // Raw sample and time of this scan: the slot readSensors() just wrote
  uint8_t slot = (vindex + VELOCITY_WINDOW - 1) % VELOCITY_WINDOW;
  captureTime[captureHead] = velocityTimes[slot];
  captureRaw[captureHead] = velocityHistory[captureKey][slot];
  captureReading[captureHead] = currSensorReadings[captureKey];
  captureNoteOn[captureHead] = (jackStates[captureKey] == PLUCK) ? 1 : 0;
  captureHead = (captureHead + 1) % CAPTURE_SIZE;

  uint16_t release = releaseThresholds[captureKey];

  if (captureState == CAPTURE_ARMED) {
    if (captureFilled < CAPTURE_SIZE)
      captureFilled++;

    // Trigger: reading falls below the release threshold (start of a press)
    if (currSensorReadings[captureKey] < release and prevSensorReadings[captureKey] >= release) {
      capturePreKept = captureFilled - 1;  // samples before the trigger sample
      if (capturePreKept > CAPTURE_PRE)
        capturePreKept = CAPTURE_PRE;
      captureLength = capturePreKept + 1;
      captureAboveCount = 0;
      captureState = CAPTURE_RECORDING;
    }
    return;
  }

  // Recording: the capture is the last captureLength samples, so it never
  // overwrites its own start as long as captureLength <= CAPTURE_SIZE
  captureLength++;

  if (currSensorReadings[captureKey] > release)
    captureAboveCount++;
  else
    captureAboveCount = 0;

  if (captureAboveCount >= CAPTURE_POST)
    finishCapture(false);
  else if (captureLength >= CAPTURE_SIZE)
    finishCapture(true);
}

/**
 * @brief Dump the finished capture over serial, then return to idle
 *
 * @param bufferFull true if the capture stopped because the buffer was full
 */
void finishCapture(bool bufferFull) {
  uint16_t start = (captureHead + CAPTURE_SIZE - captureLength) % CAPTURE_SIZE;
  unsigned long t0 = captureTime[start];

  Serial.println();
  Serial.println("[CAPTURE] BEGIN");
  Serial.print("# firmware: ");
  Serial.println(FIRMWARE_STRING);
  Serial.print("# key: ");
  Serial.println(index2key(captureKey));
  Serial.print("# sensor_index: ");
  Serial.println(captureKey);
  Serial.print("# register: ");
  Serial.println(jackRegister == FRONT_REGISTER ? "front" : "back");
  Serial.print("# pluck_threshold: ");
  Serial.println(pluckThresholds[captureKey]);
  Serial.print("# release_threshold: ");
  Serial.println(releaseThresholds[captureKey]);
  Serial.print("# avg_size: ");
  Serial.println(AVG_SIZE);
  Serial.print("# reading_scale: ");  // reading = raw average x reading_scale
  Serial.println(AVG_SIZE >> (AVG_SHIFT - 1));
  Serial.print("# velocity_window: ");
  Serial.println(VELOCITY_WINDOW);
  Serial.print("# samples: ");
  Serial.println(captureLength);
  Serial.print("# pretrigger: ");
  Serial.println(capturePreKept);
  Serial.print("# end: ");
  Serial.println(bufferFull ? "buffer_full" : "released");
  Serial.println("t_us,raw,reading,note_on");

  for (uint16_t i = 0; i < captureLength; i++) {
    uint16_t idx = (start + i) % CAPTURE_SIZE;
    Serial.print(captureTime[idx] - t0);  // unsigned: correct across micros() wrap
    Serial.print(',');
    Serial.print(captureRaw[idx]);
    Serial.print(',');
    Serial.print(captureReading[idx]);
    Serial.print(',');
    Serial.println(captureNoteOn[idx]);
  }

  Serial.println("[CAPTURE] END");
  endCapture();
}

/**
 * @brief Return to idle: restore plot printing and the key-select LED
 */
void endCapture() {
  captureState = CAPTURE_IDLE;
  shouldPrint = captureSavedPrint;

  leds.fill(leds.Color(0, 0, 0), 0, numSensors);
  leds.setPixelColor(curKeyIndex, currentLedColor);
  leds.show();
}
