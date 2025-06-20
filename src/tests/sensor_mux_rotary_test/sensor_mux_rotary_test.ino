/*

  Mux Rotary: Switch Mux Channels with rotary encoder.

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
#include <Rotary.h>
#include <Button2.h>
#include <Rotary.h>
#include <Button2.h>
#include <Adafruit_NeoPixel.h>
//-----------------------------------------------------------------------------
const byte numPcbs = 7;
const byte sensorsPerPcb = 7;
const byte numSensors = numPcbs * sensorsPerPcb;
pin_size_t sensorPins[numPcbs] = { A0, A1, A2, A3, A4, A5, A6 };
//-----------------------------------------------------------------------------
// Multiplexer Variables
const size_t muxPinA = 6;
const size_t muxPinB = 7;
const size_t muxPinC = 8;
// const size_t muxPinD = 9;
//-----------------------------------------------------------------------------
// Rotary Variables
const byte ROTARY_PINC = 12;
const byte ROTARY_PINA = 10;
const byte ROTARY_PINB = 11;
const byte CLICKS_PER_STEP = 4;
const byte MIN_POS = 0;

const uint16_t MAX_POS = 6;
const byte START_POS = 0;
const byte INCREMENT = 1;
Rotary rotary = Rotary(ROTARY_PINA, ROTARY_PINB, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
Button2 button = Button2(ROTARY_PINC);
uint8_t mux = 0;
//-----------------------------------------------------------------------------
const size_t ledPin = 9;
Adafruit_NeoPixel leds(numSensors, ledPin, NEO_GRB + NEO_KHZ800);
//-----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);


  pinMode(muxPinA, OUTPUT);
  pinMode(muxPinB, OUTPUT);
  pinMode(muxPinC, OUTPUT);
  // switch mux channel
  digitalWrite(muxPinA, (mux >> 0) & 0x1);
  digitalWrite(muxPinB, (mux >> 1) & 0x1);
  digitalWrite(muxPinC, (mux >> 2) & 0x1);

  leds.begin();
  leds.clear();
  leds.show();

  rotary.setChangedHandler(rotate);
  rotary.resetPosition(2);
}

void loop() {
  rotary.loop();
  printValues();
}

void printValues() {
  for (int i = 0; i < numPcbs; i++) {
    Serial.print(i);
    Serial.print('-');
    Serial.print(mux);
    Serial.print(':');
    Serial.print(analogRead(sensorPins[i]));
    Serial.print(' ');
  }
  Serial.print("Min:");
  Serial.print(0);
  Serial.print(' ');
  Serial.print("Max:");
  Serial.println(1024);
}


void rotate(Rotary& r) {
  mux = rotary.getPosition();
  digitalWrite(muxPinA, (mux >> 0) & 0x1);
  digitalWrite(muxPinB, (mux >> 1) & 0x1);
  digitalWrite(muxPinC, (mux >> 2) & 0x1);

  leds.clear();
  for (int i = mux; i < numSensors; i += sensorsPerPcb) {
    leds.setPixelColor(i, leds.Color(0, 100, 0));
  }
  leds.show();
}
