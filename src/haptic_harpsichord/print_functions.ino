/**
 * @file print_functions.ino
 * @brief Utilities for printing data over Serial
 *
 * Functions are for priting data to the Arduino IDE Serial monitor
 * Functions do not print a newline, which will needed to bed added manually
 * This allows the functions to be mixed and matched and used mor modularly.
 *
 * @date 2025-10-11
 * @author Matthew Hamilton
 */

// void printJackStates() {
//   bool anyPrint = false;

//   if (jackStates[0] == PLUCK and prevStates[0] == RELEASED) {
//     Serial.print("Jack 1 Plucked ");
//     anyPrint = true;
//   }
//   if (jackStates[1] == PLUCK and prevStates[1] == RELEASED) {
//     Serial.print("Jack 2 Plucked ");
//     anyPrint = true;
//   }
//   if (jackStates[2] == PLUCK and prevStates[2] == RELEASED) {
//     Serial.print("Jack 3 Plucked ");
//     anyPrint = true;
//   }
//   if (anyPrint)
//     Serial.println();
// }

//-----------------------------------------------------------------------------
/**
 * @brief Print the sensor value for the jack at a given key index 
 * 
 * Intended to be used in conjunction with the Arduino IDE serial plotter.
 * Uses the tagged data format `TAG:VALUE ` and does not terminate
 * in a new line.
 *
 * `min` and `max` are printed as constant `m`/`M` lines that fix the
 * plotter's vertical scale.
 *
 * @param index key index
 * @param min lower plot limit
 * @param max upper plot limit
 */
void printJackReading(byte index, uint16_t min, uint16_t max) {
  Serial.print("m:");
  Serial.print(min);
  Serial.print(' ');

  Serial.print("M:");
  Serial.print(max);
  // Serial.print(1024);
  Serial.print(' ');

  Serial.print("K");
  Serial.print(index2key(index));
  Serial.print(":");
  Serial.print(currSensorReadings[index]);
  Serial.print(' ');
}

/**
 * @brief Print a key's reading with plot limits that follow its thresholds
 *
 * Wraps `printJackReading()`, choosing the `m`/`M` limits so the Serial
 * Plotter window surrounds the key's threshold(s), with a margin equal to the
 * gap between pluck and release (at least `minMargin`). For an auto-calibrated
 * key this shows its whole travel, from fully pressed to rest, and selecting
 * a different key rescales the plot to that key.
 *
 * @param index key index
 */
void printJackReadingAroundThresholds(byte index) {
  const int32_t minMargin = 100;
  int32_t lo, hi;

  switch (thresholdType) {
    case SINGLE_THRESHOLD:
      lo = singlePluckThresholds[index];
      hi = singlePluckThresholds[index];
      break;
    case HYSTERETIC:
    default:
      lo = pluckThresholds[index];
      hi = releaseThresholds[index];
      if (lo > hi) {  // tolerate a mis-calibrated key
        int32_t t = lo;
        lo = hi;
        hi = t;
      }
      break;
  }

  int32_t margin = hi - lo;
  if (margin < minMargin)
    margin = minMargin;

  lo -= margin;
  hi += margin;
  if (lo < 0)
    lo = 0;
  if (hi > SENSOR_READING_MAX)
    hi = SENSOR_READING_MAX;

  printJackReading(index, lo, hi);
}

/**
 * @brief Print the threhold value for the given key index
 * 
 * Intended to be used in conjunction with the Arduino IDE serial plotter.
 * Prints in the format `T:value `
 *
 * Does no terminate in a newline
 *
 * @param index 
 */
void printJackThreshold(byte index) {
  switch (thresholdType) {
    case SINGLE_THRESHOLD:
      Serial.print("T");
      Serial.print(index2key(index));
      Serial.print(":");
      Serial.print(singlePluckThresholds[index]);
      break;
    case HYSTERETIC:
      Serial.print("P");
      Serial.print(index2key(index));
      Serial.print(":");
      Serial.print(pluckThresholds[index]);
      Serial.print(" ");
      Serial.print("R");
      Serial.print(index2key(index));
      Serial.print(":");
      Serial.print(releaseThresholds[index]);
      break;
  }
}



void printFirmwareInfo() {

  Serial.print("Firmware Version: ");
  Serial.println(FIRMWARE_STRING);

  Serial.print("Threshold Type: ");
  switch (thresholdType) {
    case SINGLE_THRESHOLD:
      Serial.println("Single");
      Serial.println("WARNING: single threshold mode is DEPRECATED and should not be used. Set thresholdType = HYSTERETIC.");
      break;
    case HYSTERETIC:
      Serial.println("Hysteretic");
      break;
  }

  Serial.print("Jack Register: ");
  switch (jackRegister) {
    case FRONT_REGISTER:
      Serial.println("Front");
      break;
    case BACK_REGISTER:
      Serial.println("Back");
      break;
    default:
      Serial.println("Unknown");
      break;
  }

  Serial.print("Mode: ");
  if (executeDebugMode)
    Serial.println("DEBUG");
  else
    Serial.println("Standard");
}