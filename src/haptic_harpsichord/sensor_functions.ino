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
 * Also updates `sensorVelocity` for every sensor: the least-squares slope of
 * a straight line fitted through the newest VELOCITY_WINDOW raw samples,
 * against their measured scan times. The samples are kept in their own ring
 * buffer (`velocityHistory`, `velocityTimes`, `vindex`), separate from the
 * moving average, so the window length does not affect the readings. It is
 * centred (VELOCITY_WINDOW - 1) / 2 scans back, and every sample in the fit
 * contributes, which gives less noise than a difference of the two end
 * samples. A shorter window gives less lag but more noise.
 */
void readSensors() {

  // Timing: measured, not a fixed per-mode value, since scan time varies with
  // mode, serial printing and MIDI traffic. Unsigned subtraction stays
  // correct when micros() wraps (~71.6 minutes).
  unsigned long now = micros();
  scanDtMicros = now - velocityTimes[(vindex + VELOCITY_WINDOW - 1) % VELOCITY_WINDOW];
  windowMicros = now - velocityTimes[(vindex + 1) % VELOCITY_WINDOW];  // oldest kept after this write
  velocityTimes[vindex] = now;

  // Least-squares slope weights, computed once per scan and shared by all
  // sensors. The fit uses all VELOCITY_WINDOW slots of velocityHistory after
  // this scan's write (slot vindex = newest). For a line through (t_i, x_i)
  // the slope is sum(w_i * x_i) with w_i = (t_i - mean t) / sum((t_j - mean t)^2).
  // Times are taken as "microseconds ago" relative to `now`, so floats keep
  // full precision; slope against time is minus the slope against age.
  // The weights also convert raw units per microsecond to 13-bit reading
  // units (readings = raw average x 2) per second.
  // Velocity is valid once this write fills the buffer.
  bool velocityValid = (velocityCount >= VELOCITY_WINDOW - 1) and (windowMicros > 0);
  float slotWeights[VELOCITY_WINDOW];
  if (velocityValid) {
    float age[VELOCITY_WINDOW];
    float meanAge = 0.0f;
    for (uint8_t s = 0; s < VELOCITY_WINDOW; s++) {
      age[s] = (float)(now - velocityTimes[s]);  // slot vindex is now: age 0
      meanAge += age[s];
    }
    meanAge /= VELOCITY_WINDOW;

    float sumSq = 0.0f;
    for (uint8_t s = 0; s < VELOCITY_WINDOW; s++)
      sumSq += (age[s] - meanAge) * (age[s] - meanAge);

    if (sumSq > 0.0f) {
      float scale = -((float)AVG_SIZE / (1 << (AVG_SHIFT - 1))) * 1000000.0f / sumSq;
      for (uint8_t s = 0; s < VELOCITY_WINDOW; s++)
        slotWeights[s] = (age[s] - meanAge) * scale;
    } else {
      velocityValid = false;
    }
  }

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

      // Velocity: store the raw sample in the velocity buffer, then take the
      // least-squares slope (raw samples: no rounding from the shift). The
      // weights sum to zero, so samples are taken relative to `raw` to keep
      // the float terms small and precise.
      velocityHistory[sensorIdx][vindex] = raw;
      if (velocityValid) {
        float v = 0.0f;
        for (uint8_t s = 0; s < VELOCITY_WINDOW; s++)
          v += slotWeights[s] * (float)((int32_t)velocityHistory[sensorIdx][s] - (int32_t)raw);
        sensorVelocity[sensorIdx] = v;
      } else {
        sensorVelocity[sensorIdx] = 0.0f;
      }
    }
  }

  windex = (windex + 1) & (AVG_SIZE - 1);  // wrap (cheap because AVG_SIZE is pow2)

  vindex = (vindex + 1) % VELOCITY_WINDOW;  // any length, not only powers of 2
  if (velocityCount < VELOCITY_WINDOW)
    velocityCount++;
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
