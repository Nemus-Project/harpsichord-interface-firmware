/**
 * @file midi_functions.h
 * @brief Utility functions for generating MIDI
 *
 * If the MIDI library or MIDI technology used is changed, those
 * changes should be applied here.
 *
 * @date 2025-10-11
 * @author Matthew Hamilton
 */

/**
 * @brief Send Note On
 *
 * @param channel  MIDI channel 4-bit
 * @param pitch  MIDI pitch 7-bit
 * @param velocity  MIDI velocity pitch 7-bit
 */
void noteOn(byte channel, byte pitch, byte velocity) {  
  MidiUSB.write(MIDIMessage::NoteOn(pitch, velocity, channel));
}

/**
 * @brief Send Note Off
 * 
 * @param channel  MIDI channel 4-bit
 * @param pitch  MIDI pitch 7-bit
 * @param velocity  MIDI velocity pitch 7-bit
 */
void noteOff(byte channel, byte pitch, byte velocity) {
  MidiUSB.write(MIDIMessage::NoteOff(pitch, velocity, channel));
}

/**
 * @brief Send Control Change Message
 * 
 * @param channel 
 * @param control 
 * @param value 
 */
void controlChange(byte channel, byte control, byte value) {
  MidiUSB.write(MIDIMessage::ControlChange(control, value, channel));
}

/**
 * @brief Send polyphonic aftertouch
 * 
 * @param channel 
 * @param key 
 * @param pressure 
 */
void aftertouch(byte channel, byte key, byte pressure) {
  MidiUSB.write(MIDIMessage::PolyphonicAftertouch(key, pressure, channel));
}

/**
 * @brief Translate between key index and note.
 * 
 * Overrides for short octave or transposition should happen here
 * Currently hard cded to start from C3
 *
 * @param index key index
 * @param transpose 
 * @return byte 
 */
byte index2note(byte index, byte transpose)
{
  return numSensors - 1 - index + 48 + transpose;
}
