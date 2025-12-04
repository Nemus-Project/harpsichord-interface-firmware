//-----------------------------------------------------------------------------
// EEPROM Functions

/// Read Data from EEPROM if the DATA tag is present in memory. If not, this
/// must be a first in which case use the default values for pluckValues and
/// write that to memory.
void readPluckFromEEPROM() {

  uint32_t tagRead;

  fram.read(tagAddress, (uint8_t*)&tagRead, 4);

  if (tagRead != *((uint32_t*)thresholdTag)) {
    halt(FRAM_NOT_FOUND);
    // fram.write(tagAddress, thresholdTag, 4);
    // fram.write(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * 4);
  } else {
    fram.read(pluckValAddress, (uint8_t*)pluckThresholds, numSensors * 2);
  }
}

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