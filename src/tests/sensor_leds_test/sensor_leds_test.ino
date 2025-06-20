/*
  Haptic Harpsichord: Snesor LED Test

  Show the state of all sensors on their corresponding LED

  | Connection      | Pin   |
  | --------------- | ----- |
  | NC              | D13   |
  | EEPROM VCC      | 3.3v  |
  | AREF POT Switch | AREF  |
  | PCB 0 QRE1113   | A0    |
  | PCB 1 QRE1113   | A1    |
  | PCB 2 QRE1113   | A2    |
  | PCB 3 QRE1113   | A3    |
  | PCB 4 QRE1113   | A4    |
  | PCB 5 QRE1113   | A6    |
  | PCB 6 QRE1113   | A7    |
  | NC              | 5v    |
  | NC              | RESET |
  | GND             | GND   |
  | PSU `+`         | VIN   |

  | Connection      | Pin     |
  | --------------- | ------- |
  | Rotary CLK      | D12     |
  | Rotary DATA     | D11     |
  | Rotary Switch   | D10     |
  | LED Data        | D9      |
  | Mux C           | D8      |
  | Mux B           | D7      |
  | Mux A           | D6      |
  | EEPROM SPI CLK  | D5      |
  | EEPROM SPI MISO | D4      |
  | EEPROM SPI MOSI | D3      |
  | EEPROM SPI CS   | D2      |
  | NC              | RX / D1 |
  | NC              | TX / D0 |

 */
//-----------------------------------------------------------------------------
// Board specific libraries
#ifdef ARDUINO_ARDUINO_NANO33BLE
#include <PluggableUSBMIDI.h>
#elif ESP32
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>  // verision 3.1.3
#include <MIDI.h>
#elif ARDUINO_ARCH_STM32
#endif

#define numSensors 49
#define numMuxChannels 7
#define numPcbs 7
#define key_num_45 0;
//-----------------------------------------------------------------------------
// Board Agnostics Libraries
#include <Rotary.h>
#include <Button2.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_FRAM_SPI.h>
#include <SPI.h>
//-----------------------------------------------------------------------------
const size_t ledPin = 9;
Adafruit_NeoPixel leds(numSensors, ledPin, NEO_GRB + NEO_KHZ800);
uint8_t currSensorReadings[numSensors];
//-----------------------------------------------------------------------------
// Multiplexer Variables
const size_t muxPinA = 6;
const size_t muxPinB = 7;
const size_t muxPinC = 8;
// const size_t muxPinD = 9;


void setup() {
  leds.begin();
  leds.clear();
  leds.show();
}

void loop() {
  readSensors();
  setLeds();
}

/// Read all sensors and put results in currSensorReadings
void readSensors() {

  for (int mux = 0; mux < numMuxChannels; mux++) {

    // switch mux channel
    digitalWrite(muxPinA, (mux >> 0) & 0x1);
    digitalWrite(muxPinB, (mux >> 1) & 0x1);
    digitalWrite(muxPinC, (mux >> 2) & 0x1);
    // digitalWrite(muxPinD, (mux >> 3) & 0x1);

    for (int adc = 0; adc < numPcbs; adc++) {
      int i = mux + (adc * numPcbs);
      currSensorReadings[i] = (analogRead(A0 + adc) >> 2);
    }
  }
}

void setLeds() {

  for (int i = 0; i < numSensors; i++)
    leds.setPixelColor(i, leds.Color(currSensorReadings[i], 0, 0));  // Set pixel 'c' to value 'color'

  leds.show();
}
