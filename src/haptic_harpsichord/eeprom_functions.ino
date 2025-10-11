/**
 * @file eeprom_functions.ino
 * @brief Functions for readingg and writing to EEPROM / NV RAM
 *
 * Read Data from EEPROM if the DATA tag is present in memory. If not, this
 * must be a first in which case use the default values for pluckValues and
 * write that to memory.
 *
 * @date 2025-10-11
 * @author Matthew Hamilton
 */

/**
 * @brief Read pluck thresholds from memory
 * 
 * If the pluck threhold tag `tagAddress` is not found at the start, 
 * write it and populate the memory block with default values.
 */
void readPluckFromEEPROM() {

  uint32_t tagRead;

  fram.read(tagAddress, (uint8_t*)&tagRead, 4);

  if (tagRead != *((uint32_t*)thresholdTag)) {
    // fram.write(tagAddress, thresholdTag, 4);
    // fram.write(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * 4);
  } else {
    fram.read(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * 2);
  }
}

/**
 * @brief Write current threshold values to memory 
 * 
 * On `FRAM_WRITE_FAIL` of either tag or thresholds fails, the onboard RED LED will turn on
 */
void writePluckToEEPROM() {
  fram.writeEnable(true);

  if (!fram.write(tagAddress, thresholdTag, 4))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(true);
  if (!fram.write(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * 2))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(false);
}

// -----------------------------------------------------------------------------