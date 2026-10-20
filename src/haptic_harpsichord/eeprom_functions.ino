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

  // Single Threshold
  fram.read(singleThresholdTagAddress, (uint8_t*)&tagRead, 4);

  if (tagRead != *((uint32_t*)singleThresholdTag)) {
    fram.writeEnable(true);  // the FRAM clears write enable after every write
    fram.write(singleThresholdTagAddress, singleThresholdTag, 4);

    for (int i = 0; i < numSensors; i++)
      singlePluckThresholds[i] = DEFAULT_THRESHOLD;

    fram.writeEnable(true);
    fram.write(thresholdValueAddress, (uint8_t*)singlePluckThresholds, numSensors * 2);
  } else {
    fram.read(thresholdValueAddress, (uint8_t*)singlePluckThresholds, numSensors * 2);
  }

  // Hysteresis Pluck Threshold
  fram.read(pluckThresholdTagAddress, (uint8_t*)&tagRead, 4);

  if (tagRead != *((uint32_t*)pluckTag)) {
    fram.writeEnable(true);  // the FRAM clears write enable after every write
    fram.write(pluckThresholdTagAddress, pluckTag, 4);

    for (int i = 0; i < numSensors; i++)
      pluckThresholds[i] = DEFAULT_THRESHOLD;

    fram.writeEnable(true);
    fram.write(pluckValueAddress, (uint8_t*)pluckThresholds, numSensors * 2);
  } else {
    fram.read(pluckValueAddress, (uint8_t*)pluckThresholds, numSensors * 2);
  }

  // Hysteresis Release Threshold
  fram.read(releaseThresholdTagAddress, (uint8_t*)&tagRead, 4);

  if (tagRead != *((uint32_t*)releaseTag)) {
    fram.writeEnable(true);  // the FRAM clears write enable after every write
    fram.write(releaseThresholdTagAddress, releaseTag, 4);

    for (int i = 0; i < numSensors; i++)
      releaseThresholds[i] = DEFAULT_THRESHOLD;

    fram.writeEnable(true);
    fram.write(releaseValueAddress, (uint8_t*)releaseThresholds, numSensors * 2);
  } else {
    fram.read(releaseValueAddress, (uint8_t*)releaseThresholds, numSensors * 2);
  }

  // Register Select
  fram.read(registerTypeTagAddress, (uint8_t*)&tagRead, 4);

  if (tagRead != *((uint32_t*)registerTypeTag)) {
    setRegister();
    writeRegisterToEEPROM();
  } else {    
    fram.read(registerTypeAddress, (uint8_t*)(&jackRegister), sizeof(jackRegister));
  }

  fram.writeEnable(false);
}

/**
 * @brief Write current threshold values to memory 
 * 
 * On `FRAM_WRITE_FAIL` of either tag or thresholds fails, the onboard RED LED will turn on
 */
void writePluckToEEPROM() {

  fram.writeEnable(true);
  if (!fram.write(thresholdValueAddress, (uint8_t*)singlePluckThresholds, numSensors * 2))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(true);
  if (!fram.write(pluckValueAddress, (uint8_t*)pluckThresholds, numSensors * 2))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(true);
  if (!fram.write(releaseValueAddress, (uint8_t*)releaseThresholds, numSensors * 2))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(false);
}

// -----------------------------------------------------------------------------

/**
 * @brief Write the jack register and its tag to memory
 *
 * The FRAM clears its write-enable latch after every write, so write enable
 * is set before each one. On failure the onboard RED LED turns on.
 */
void writeRegisterToEEPROM() {

  fram.writeEnable(true);
  if (!fram.write(registerTypeTagAddress, registerTypeTag, 4))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(true);
  if (!fram.write(registerTypeAddress, (uint8_t*)(&jackRegister), sizeof(jackRegister)))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(false);
}
