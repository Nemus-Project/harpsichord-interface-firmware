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

void plotKeyData(byte index) {
  Serial.print("m:");
  Serial.print(0);
  Serial.print(' ');

  Serial.print("M:");
  Serial.print(1024);
  Serial.print(' ');
  
  Serial.print("K");
  Serial.print(index2key(index));
  Serial.print(":");
  Serial.print(currSensorReadings[index]);
  Serial.print(' ');
  
  Serial.print("T");
  Serial.print(index2key(index));
  Serial.print(":");
  Serial.println(pluckThresholds[index]);
}
