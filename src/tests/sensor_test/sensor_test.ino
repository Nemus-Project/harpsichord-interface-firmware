/**
 * @file SensorTest.ino 
 * @brief Test a single sensor on each board
 * 
 * ## Pinout
 *
 * |     Connection | Pin Left | Pin Right | Connection      |
 * | -------------: | :------- | --------: | :-------------- |
 * | EEPROM SPI CLK |      D13 | D12       | Rotary CLK      |
 * |     EEPROM VCC |     3.3v | D11       | Rotary DATA     |
 * |             NC |     AREF | D10       | Rotary Switch   |
 * |          PCB 0 |       A0 | D9        | LED Data        |
 * |          PCB 1 |       A1 | D8        | Mux C           |
 * |          PCB 2 |       A2 | D7        | Mux B           |
 * |          PCB 3 |       A3 | D6        | Mux A           |
 * |          PCB 4 |       A4 | D5        | Mux Interrupt   |
 * |          PCB 5 |       A5 | D4        | EEPROM SPI MISO |
 * |          PCB 6 |       A6 | D3        | EEPROM SPI MOSI |
 * |             NC |       A7 | D2        | EEPROM SPI CS   |
 * |             NC |       5v | GND       | GND             |
 * |             NC |    RESET | RST       | Reset Switch    |
 * |            GND |      GND | RX / D1   | NC              |
 * |        PSU `+` |      VIN | TX / D0   | NC              |
 *
 *
 */

const byte numSensors = 6;
pin_size_t sensorPins[numSensors] = { A0, A1, A2, A3, A6, A7 };
unsigned long sensorHomeValues[numSensors];
int sensorValues[numSensors];
//-----------------------------------------------------------------------------
// Multiplexer Variables
/// Mux Channel Set Pin A
const size_t muxPinA = 6;
/// Mux Channel Set Pin B
const size_t muxPinB = 7;
/// Mux Channel Set Pin C
const size_t muxPinC = 8;
// const size_t muxPinD = 9;

void setup() {
  /// setup mux
  pinMode(muxPinA, OUTPUT);
  pinMode(muxPinB, OUTPUT);
  pinMode(muxPinC, OUTPUT);


  Serial.begin(9600);
  analogReadResolution(12);
}

void loop() {
  printValues();
}

void printValues() {
  for (int i = 0; i < 7; i++) {
    
    digitalWrite(muxPinA, (i >> 0) & 0x1);
    digitalWrite(muxPinB, (i >> 1) & 0x1);
    digitalWrite(muxPinC, (i >> 2) & 0x1);

    Serial.print(i);
    Serial.print(':');
    Serial.print(analogRead(A5));
    Serial.print(' ');
  }
  Serial.print("Min:");
  Serial.print(0);
  Serial.print(' ');
  Serial.print("Max:");
  Serial.println(1300);
}
