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
 * Normalising lines for a 10-bit DAC are hardcoded
 *
 * @param index
 */
void printJackReading(byte index) {
  Serial.print("m:");
  Serial.print(750);
  Serial.print(' ');

  Serial.print("M:");
  Serial.print(1200);
  // Serial.print(1024);
  Serial.print(' ');

  Serial.print("K");
  Serial.print(index2key(index));
  Serial.print(":");
  Serial.print(readSensor(index));
  Serial.print(' ');
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