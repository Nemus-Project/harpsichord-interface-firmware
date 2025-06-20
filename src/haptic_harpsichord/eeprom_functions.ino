//-----------------------------------------------------------------------------
// EEPROM Functions

/// Read Data from EEPROM if the threshold tags are present in memory.
/// If not, this must be a first write, in which case use the default values for pluckValues and releaseValues
/// write that to memory.
/// Otherwise, read memory into threshold value arrays

void readPluckFromEEPROM() {

  uint32_t tagRead = 0;

  fram.read(pluckTagAddress, (uint8_t*)&tagRead, 4);

  if (tagRead != *((uint32_t*)pluckTag) || overwriteFram) {
    for (int i = 0; i < numSensors; i++)
      pluckThresholds[i] = defaultPluckThreshold;

    fram.writeEnable(true);
    Serial.println("Overwriting DATA Tag");
    fram.write(pluckTagAddress, pluckTag, 4);
    fram.write(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * 2);
    fram.writeEnable(false);

  } else {
    fram.read(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * 2);
  }

  tagRead = 0;

  if (tagRead != *((uint32_t*)releaseTag) || overwriteFram) {
    for (int i = 0; i < numSensors; i++)
      releaseThresholds[i] = defaultReleaseThreshold;

    fram.writeEnable(true);
    Serial.println("Overwriting DATA Tag");
    fram.write(pluckTagAddress, pluckTag, 4);
    fram.write(releaseValAddress, (uint8_t*)releaseThresholds, numSensors * 2);
    fram.writeEnable(false);

  } else {
    fram.read(releaseValAddress, (uint8_t*)releaseThresholds, numSensors * 2);
  }
}

void writePluckToEEPROM() {
  
  fram.writeEnable(true);
  if (!fram.write(pluckTagAddress, pluckTag, 4))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(true);
  if (!fram.write(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * sizeof(pluckThresholds[0])))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(true);
  if (!fram.write(releaseTagAddress, releaseTag, 4))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(true);
  if (!fram.write(releaseValAddress, (uint8_t*)releaseThresholds, numSensors * sizeof(releaseThresholds[0])))
    digitalWrite(LEDR, LOW);

  fram.writeEnable(false);
}



// -----------------------------------------------------------------------------