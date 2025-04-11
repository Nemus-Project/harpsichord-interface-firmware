void noteOn(byte channel, byte pitch, byte velocity) {  
  MidiUSB.write(MIDIMessage::NoteOn(pitch, velocity, channel));
}

void noteOff(byte channel, byte pitch, byte velocity) {
  MidiUSB.write(MIDIMessage::NoteOff(pitch, velocity, channel));
}

void controlChange(byte channel, byte control, byte value) {
  MidiUSB.write(MIDIMessage::ControlChange(control, value, channel));
}

void aftertouch(byte channel, byte key, byte pressure) {
  MidiUSB.write(MIDIMessage::PolyphonicAftertouch(key, pressure, channel));
}

byte index2note(byte index, byte transpose)
{
  return numSensors - 1 - index + 48 + transpose;
}
