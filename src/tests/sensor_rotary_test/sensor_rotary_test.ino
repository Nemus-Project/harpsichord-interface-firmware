/*

  Choose a single sensor and plot the data

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
const byte numSensors = 49;
const byte numPcbs = 7;
const byte numMuxChannels = 7;
size_t sensorPins[numPcbs] = { A0, A1, A2, A3, A4, A5, A6 };
int currSensorReadings[numSensors];
uint8_t curKeyIndex = 0;
//-----------------------------------------------------------------------------
// Multiplexer Variables
const size_t muxPinA = 6;
const size_t muxPinB = 7;
const size_t muxPinC = 8;
// const size_t muxPinD = 9;
//-----------------------------------------------------------------------------
const size_t ledPin = 9;
Adafruit_NeoPixel leds(numSensors, ledPin, NEO_GRB + NEO_KHZ800);
//-----------------------------------------------------------------------------
// Rotary Variables
const byte ROTARY_PINC = 12;
const byte ROTARY_PINA = 10;
const byte ROTARY_PINB = 11;
const byte CLICKS_PER_STEP = 4;
const byte MIN_POS = 0;

const uint16_t MAX_POS = numSensors - 1;
const byte START_POS = 0;
const byte INCREMENT = 1;
Rotary rotary = Rotary(ROTARY_PINA, ROTARY_PINB, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
Button2 button = Button2(ROTARY_PINC);
//-----------------------------------------------------------------------------
int adc = 0;
int mux = 0;
uint16_t currentSensorValues[numSensors];
//-----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  pinMode(muxPinA, OUTPUT);
  pinMode(muxPinB, OUTPUT);
  pinMode(muxPinC, OUTPUT);

  leds.begin();
  rotary.setChangedHandler(rotate);
  rotary.resetPosition(25);
}

void loop() {
  rotary.loop();
  printValues();
}

void printValues() {
  readAllSensors();
  int i = curKeyIndex;
  Serial.print(i);
  Serial.print(':');
  Serial.print(currentSensorValues[curKeyIndex]);
  Serial.print(' ');

  Serial.print("Min:");
  Serial.print(0);
  Serial.print(' ');
  Serial.print("Max:");
  Serial.println(1024);
}

//-----------------------------------------------------------------------------
// Rotary Callbacks
void rotate(Rotary& r) {
  curKeyIndex = rotary.getPosition();
  adc = curKeyIndex / numPcbs;
  mux = curKeyIndex - adc * numPcbs;

  digitalWrite(muxPinA, (mux >> 0) & 0x1);
  digitalWrite(muxPinB, (mux >> 1) & 0x1);
  digitalWrite(muxPinC, (mux >> 2) & 0x1);

  leds.clear();
  leds.setPixelColor(rotary.getPosition(), leds.Color(0, 100, 0));
  leds.show();
}


void readAllSensors() {

  for (uint8_t i = 0; i < 7; i++) {
    
    digitalWrite(muxPinA, (i >> 0) & 0x1);
    digitalWrite(muxPinB, (i >> 1) & 0x1);
    digitalWrite(muxPinC, (i >> 2) & 0x1);

    delayMicroseconds(500);

    currentSensorValues[i + (0 * 7)] = analogRead(A0 + 0);
    currentSensorValues[i + (1 * 7)] = analogRead(A0 + 1);
    currentSensorValues[i + (2 * 7)] = analogRead(A0 + 2);
    currentSensorValues[i + (3 * 7)] = analogRead(A0 + 3);
    currentSensorValues[i + (4 * 7)] = analogRead(A0 + 4);
    currentSensorValues[i + (5 * 7)] = analogRead(A0 + 5);
    currentSensorValues[i + (6 * 7)] = analogRead(A0 + 6);
  }
}
