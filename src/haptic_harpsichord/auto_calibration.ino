/**
 * @file auto_calibration.ino
 * @brief Automatic per-key threshold calibration
 *
 * @date 2026-04-16
 */

/**
 * @brief Auto-calibrate pluckThresholds and releaseThresholds for every sensor.
 *
 * Three phases:
 *  1. REST     — measures idle mean and noise floor (user must not touch keyboard)
 *  2. ACTIVITY — captures minimum reading per sensor while user plays each key
 *  3. COMPUTE  — sets pluck at 25% and release at 75% of observed travel per key
 *
 * Keys that show no meaningful travel during activity are left untouched and
 * flagged yellow on the LED strip. Results are held in RAM — use the encoder
 * double-click (writePluckToEEPROM) to commit to FRAM once you're happy.
 */
void autoCalibrate() {

  const uint16_t REST_SAMPLES    = 500;    // idle observation window
  const unsigned long ACTIVITY_MS = 30000; // max time for user to play keys
  const uint16_t NOISE_FLOOR     = 8;      // assumed min noise if we measure less
  const uint8_t  MIN_TRAVEL_MULT = 5;      // travel must exceed this × noise to count

  uint16_t restMean[numSensors];
  uint16_t noise[numSensors];
  uint16_t minSeen[numSensors];

  // ---------- Phase 1: rest ----------
  Serial.println(F("[AUTOCAL] 1/3 RESTING — hands off keyboard"));
  leds.fill(leds.Color(30, 0, 30), 0, numSensors);   // magenta: don't touch
  leds.show();
  delay(800);                                        // give user time to lift hands

  uint32_t sum[numSensors];
  uint16_t mn[numSensors], mx[numSensors];
  for (uint8_t i = 0; i < numSensors; i++) {
    sum[i] = 0;  mn[i] = 0xFFFF;  mx[i] = 0;
  }

  for (uint16_t s = 0; s < REST_SAMPLES; s++) {
    readSensors();
    for (uint8_t i = 0; i < numSensors; i++) {
      uint16_t v = currSensorReadings[i];
      sum[i] += v;
      if (v < mn[i]) mn[i] = v;
      if (v > mx[i]) mx[i] = v;
    }
  }
  for (uint8_t i = 0; i < numSensors; i++) {
    restMean[i] = sum[i] / REST_SAMPLES;
    noise[i]    = max((uint16_t)(mx[i] - mn[i]), NOISE_FLOOR);
    minSeen[i]  = restMean[i];
  }

  // ---------- Phase 2: activity ----------
  Serial.println(F("[AUTOCAL] 2/3 PLAY — press each key a few times"));
  Serial.println(F("[AUTOCAL]       send any char to finish early"));
  leds.fill(leds.Color(0, 30, 0), 0, numSensors);    // dim green: play now
  leds.show();

  clearSerialInputBuffer();
  unsigned long start = millis();
  while ((millis() - start) < ACTIVITY_MS) {
    readSensors();
    for (uint8_t i = 0; i < numSensors; i++) {
      if (currSensorReadings[i] < minSeen[i]) {
        minSeen[i] = currSensorReadings[i];
        leds.setPixelColor(i, leds.Color(0, 100, 0));  // bright green: got it
      }
    }
    leds.show();
    if (Serial.available()) { clearSerialInputBuffer(); break; }
  }

  // ---------- Phase 3: compute ----------
  Serial.println(F("[AUTOCAL] 3/3 COMPUTING"));
  uint8_t skipped = 0;

  for (uint8_t i = 0; i < numSensors; i++) {
    uint16_t travel = (restMean[i] > minSeen[i]) ? (restMean[i] - minSeen[i]) : 0;

    if (travel < MIN_TRAVEL_MULT * noise[i]) {
      Serial.print(F("[AUTOCAL]   key "));
      Serial.print(index2key(i));
      Serial.println(F(" not pressed — kept existing values"));
      leds.setPixelColor(i, leds.Color(40, 30, 0));  // yellow: skipped
      skipped++;
      continue;
    }

    uint16_t pluck   = minSeen[i] + (travel >> 2);          // 25% up from pressed
    uint16_t release = minSeen[i] + ((travel * 3) >> 2);    // 75% up from pressed

    // Enforce a sane gap even on shallow keys
    if (release - pluck < 3 * noise[i]) {
      release = pluck + 3 * noise[i];
    }

    pluckThresholds[i]   = pluck;
    releaseThresholds[i] = release;
  }

  leds.show();
  Serial.print(F("[AUTOCAL] done — "));
  Serial.print(numSensors - skipped);
  Serial.print(F("/"));
  Serial.print(numSensors);
  Serial.println(F(" keys calibrated"));
  Serial.println(F("[AUTOCAL] double-click encoder to save to FRAM"));
}