/**
 * @file sensor_functions.ino
 * @brief Utilities for reading and interpreting sensor data
 *
 * @date 2025-10-11
 * @author Matthew Hamilton
 */

/**
 * @brief Round Robin read of sensors
 *
 * Sensor values are stored in the `currSensorReadings` array.
 *
 * A pointer swap is implemented so that `currSensorReadings` and
 * `prevSensorReadings` switch every time the function is called. The swap
 * happens at the TOP of the function so that on exit, `currSensorReadings`
 * holds the most recent reading and `prevSensorReadings` holds the one before
 * it — matching the variable names.
 *
 * A per-sensor moving average filter is applied using the global variables
 * `sensorHistory`, `sensorSums`, and `windex`. Window length is `AVG_SIZE`
 * (must be a power of 2 so division becomes a right shift by `AVG_SHIFT`).
 *
 * Also updates `sensorVelocity` for every sensor: the slope over the window,
 * (newest raw sample - raw sample AVG_SIZE scans ago) divided by the measured
 * time between those two scans (`windowMicros`).
 */
void readSensors() {

  // Timing: measured, not a fixed per-mode value, since scan time varies with
  // mode, serial printing and MIDI traffic. scanTimes[windex] still holds the
  // time of the scan AVG_SIZE scans ago, matching sensorHistory[][windex].
  // Unsigned subtraction stays correct when micros() wraps (~71.6 minutes).
  unsigned long now = micros();
  scanDtMicros = now - scanTimes[(windex - 1) & (AVG_SIZE - 1)];
  windowMicros = now - scanTimes[windex];
  scanTimes[windex] = now;

  // Velocity is valid once the window holds AVG_SIZE real scans. Raw deltas
  // are converted to the 13-bit reading scale (readings = raw average x 2)
  // and to units per second with one division per scan; float avoids the
  // overflow of (delta * 1000000) in 32-bit integers.
  bool velocityValid = (scanCount >= AVG_SIZE) and (windowMicros > 0);
  float velocityScale = 0.0f;
  if (velocityValid)
    velocityScale = ((float)AVG_SIZE / (1 << (AVG_SHIFT - 1))) * 1000000.0f / windowMicros;

  // Swap first: old 'curr' becomes 'prev', then we overwrite 'curr' with fresh data.
  tempPointer = prevSensorReadings;
  prevSensorReadings = currSensorReadings;
  currSensorReadings = tempPointer;

  for (uint8_t i = 0; i < numMuxChannels; i++) {

    digitalWrite(muxPinA, (i >> 0) & 0x1);
    digitalWrite(muxPinB, (i >> 1) & 0x1);
    digitalWrite(muxPinC, (i >> 2) & 0x1);

    // delayMicroseconds(500);

    for (uint8_t adc = 0; adc < numPcbs; adc++) {
      uint8_t sensorIdx = i + (adc * numMuxChannels);
      uint16_t raw = analogRead(A0 + adc);
      uint16_t oldest = sensorHistory[sensorIdx][windex];

      // Ring-buffer moving average: drop oldest, add newest
      sensorSums[sensorIdx] -= oldest;
      sensorHistory[sensorIdx][windex] = raw;
      sensorSums[sensorIdx] += raw;

      // Oversampling: summing AVG_SIZE samples gives log2(AVG_SIZE) extra
      // bits in sensorSums. Shifting by AVG_SHIFT-1 (not AVG_SHIFT) keeps
      // one of those bits, so currSensorReadings is 13-bit instead of 12.
      currSensorReadings[sensorIdx] = sensorSums[sensorIdx] >> (AVG_SHIFT - 1);

      // Slope over the window, from raw samples (no rounding from the shift)
      sensorVelocity[sensorIdx] = velocityValid ? (float)((int32_t)raw - (int32_t)oldest) * velocityScale : 0.0f;
    }
  }

  windex = (windex + 1) & (AVG_SIZE - 1);  // wrap (cheap because AVG_SIZE is pow2)
  if (scanCount < AVG_SIZE)
    scanCount++;
}

//-----------------------------------------------------------------------------
/**
 * @brief Read a single sensor and return its raw (unfiltered) value
 *
 * Bypasses the moving average filter. Useful for diagnostics only — do not
 * use this value for threshold decisions in the main loop.
 *
 * @param index key index
 * @return raw value of sensor at key index
 */
unsigned int readSensor(byte index) {
  int adc = index / numPcbs;
  int mux = index - adc * numPcbs;

  digitalWrite(muxPinA, (mux >> 0) & 0x1);
  digitalWrite(muxPinB, (mux >> 1) & 0x1);
  digitalWrite(muxPinC, (mux >> 2) & 0x1);
  // digitalWrite(muxPinD, (mux >> 3) & 0x1);

  return analogRead(A0 + adc);
}

//-----------------------------------------------------------------------------

/**
 * @brief Get the On Velocity for a Note On Message
 *
 * Logic for determining Note On velocity given the difference readings of
 * sensor. During a press the signal DROPS, so the new reading is lower than
 * the previous: use `prev - curr` to get a positive magnitude of motion.
 *
 * @param j key index
 * @return byte 7-bit velocity value
 */
byte getOnVelocity(int j) {
  byte velocity;
  int delta = prevSensorReadings[j] - currSensorReadings[j];

  if (j < 3) {
    velocity = map(delta, 1, VELOCITY_DELTA_MAX, 1, 63);
    if (velocity < 1)
      velocity = 1;
    else if (velocity > 63)
      velocity = 63;
  } else {
    velocity = map(delta, 1, VELOCITY_DELTA_MAX, 65, 127);
    if (velocity < 65)
      velocity = 65;
    else if (velocity > 127)
      velocity = 127;
  }

  return velocity;
}

//-----------------------------------------------------------------------------

/**
 * @brief Get the Off Velocity for a Note Off Message
 *
 * Logic for determining Note Off velocity given the difference readings of
 * sensor. During a release the signal RISES, so the new reading is higher
 * than the previous: `curr - prev` already gives a positive magnitude.
 *
 * @param j key index
 * @return byte 7-bit velocity value
 */
byte getOffVelocity(int j) {
  byte velocity;
  int delta = currSensorReadings[j] - prevSensorReadings[j];

  if (j < 3) {
    velocity = map(delta, 1, VELOCITY_DELTA_MAX, 1, 63);
    if (velocity < 1)
      velocity = 1;
    else if (velocity > 63)
      velocity = 63;
  } else {
    velocity = map(delta, 1, VELOCITY_DELTA_MAX, 65, 127);
    if (velocity < 65)
      velocity = 65;
    else if (velocity > 127)
      velocity = 127;
  }

  return velocity;
}
