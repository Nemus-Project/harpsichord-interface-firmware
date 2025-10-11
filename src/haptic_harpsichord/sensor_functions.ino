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

  for (int mux = 0; mux < numMuxChannels; mux++) {

    // switch mux channel
    digitalWrite(muxPinA, (mux >> 0) & 0x1);
    digitalWrite(muxPinB, (mux >> 1) & 0x1);
    digitalWrite(muxPinC, (mux >> 2) & 0x1);
    // digitalWrite(muxPinD, (mux >> 3) & 0x1);

    for (int adc = 0; adc < numPcbs; adc++) {

      int i = mux + (adc * numPcbs);

      sensorWindowReadings[i][windex] = analogRead(A0 + adc);

      uint16_t total = 0;

      for (int avg = 0; avg < avgSize; avg++) {
        total += sensorWindowReadings[i][avg];
      }

      currSensorReadings[i] = total / avgSize;
      // currSensorReadings[i] = analogRead(A0 + adc);
    }
  }
  
  windex++;
  if (windex >= avgSize) {
    windex = 0;
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
