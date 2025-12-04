/**
 * @file sensor_functions.ino
 * @brief Utilities for reading and interpreting sensor data
 *
 * @date 2025-10-11
 * @author Matthew Hamilton
 */

/**
 * @brief Round Robin read of sensors
 * 
 * Sensor values are stored in the `currSensorReadings` array
 *
 * A pointer swap is implemented so that `currSensorReadings` and `prevSensorReadings` switch
 * everytime the function is called. 
 *
 * A moving average filter is applied using the global variabl `avgSize` to denote window length.
 * The averaging window index `windex` is incremented on each call.
 */
void readSensors() {

  for (uint8_t i = 0; i < 7; i++) {
    
    digitalWrite(muxPinA, (i >> 0) & 0x1);
    digitalWrite(muxPinB, (i >> 1) & 0x1);
    digitalWrite(muxPinC, (i >> 2) & 0x1);

    delayMicroseconds(500);

    currSensorReadings[i + (0 * 7)] = analogRead(A0 + 0);
    currSensorReadings[i + (1 * 7)] = analogRead(A0 + 1);
    currSensorReadings[i + (2 * 7)] = analogRead(A0 + 2);
    currSensorReadings[i + (3 * 7)] = analogRead(A0 + 3);
    currSensorReadings[i + (4 * 7)] = analogRead(A0 + 4);
    currSensorReadings[i + (5 * 7)] = analogRead(A0 + 5);
    currSensorReadings[i + (6 * 7)] = analogRead(A0 + 6);
  }

  // pointer swap
  tempPointer = prevSensorReadings;
  prevSensorReadings = currSensorReadings;
  currSensorReadings = tempPointer;
}

//-----------------------------------------------------------------------------
/**
 * @brief Read a single sensor and return its raw value
 * 
 * @param index key index
 * @return value of sensor at key index
 */
unsigned int readSensor(byte index) {
  int adc = index / numPcbs;
  int mux = index - adc * numPcbs;

  digitalWrite(muxPinA, (mux >> 0) & 0x1);
  digitalWrite(muxPinB, (mux >> 1) & 0x1);
  digitalWrite(muxPinC, (mux >> 2) & 0x1);
  // digitalWrite(muxPinD, (mux >> 3) & 0x1);

  return analogRead(A0 + adc);
}

//-----------------------------------------------------------------------------

/**
 * @brief Get the On Velocity for a Note On Message
 * 
 * Logic for determining Note On velocity given the difference readings of sensor
 *
 * @param j key index
 * @return byte 7-bit velocity value 
 */
byte getOnVelocity(int j) {

  byte velocity;
  if (j < 3) {

    velocity = map(currSensorReadings[j] - prevSensorReadings[j], 1, 180, 1, 63);
    if (velocity < 1)
      velocity = 1;
    else if (velocity > 63)
      velocity = 63;

  } else {
    {
      velocity = map(currSensorReadings[j] - prevSensorReadings[j], 1, 180, 65, 127);
      if (velocity < 65)
        velocity = 65;
      else if (velocity > 127)
        velocity = 127;
    }
  }
}

//-----------------------------------------------------------------------------

/**
 * @brief Get the Off Velocity object
 *
 * Logic for determining Note Off velocity given the difference readings of sensor
 *
 * @param j key index
 * @return byte 7-bit velocity value 
 */
byte getOffVelocity(int j) {
  byte velocity;
  if (j < 3) {

    velocity = map(currSensorReadings[j] - prevSensorReadings[j], 1, 180, 1, 63);
    if (velocity < 1)
      velocity = 1;
    else if (velocity > 63)
      velocity = 63;

  } else {
    {
      velocity = map(currSensorReadings[j] - prevSensorReadings[j], 1, 180, 65, 127);
      if (velocity < 65)
        velocity = 65;
      else if (velocity > 127)
        velocity = 127;
    }
  }
}
