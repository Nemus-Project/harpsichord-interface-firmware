//-----------------------------------------------------------------------------
// EEPROM Functions

/// Read Data from EEPROM if the DATA tag is present in memory. If not, this
/// must be a first in which case use the default values for pluckValues and
/// write that to memory.
void readPluckFromEEPROM() {

  uint32_t tagRead;
  EEPROM.get<uint32_t>(tagAddress, tagRead);

  if (tagRead != *((uint32_t*)tagName)) {
    EEPROM.put<char[4]>(tagAddress, tagName);
    EEPROM.put<byte[3]>(pluckValAddress, pluckValues);

    if (!EEPROM.getCommitASAP())
      EEPROM.commit();

  } else {
    EEPROM.get<byte[3]>(pluckValAddress, pluckValues);
  }
}

void writePluckToEEPROM() {

  EEPROM.put<byte[3]>(pluckValAddress, pluckValues);

  if (!EEPROM.getCommitASAP())
    EEPROM.commit();
}
//-----------------------------------------------------------------------------