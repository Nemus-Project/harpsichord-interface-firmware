/**
 * @file rotary_test.ino
 * @brief Rotary Encoder Diagnostic
 *
 * Messages for the rotary encoder callbacks sent over Serial.
 *
 * |     Connection | Pin Left | Pin Right | Connection      |
 * | -------------: | -------: | :-------- | :-------------- |
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
 */

//-----------------------------------------------------------------------------
#include <Rotary.h>
#include <Button2.h>
//-----------------------------------------------------------------------------
const byte ROTARY_PINC = 12;
const byte ROTARY_PINA = 10;
const byte ROTARY_PINB = 11;
const byte CLICKS_PER_STEP = 4;
const byte MIN_POS = 0;
const uint16_t numSensors = 49;
const uint16_t MAX_POS = numSensors - 1;
const byte START_POS = 0;
const byte INCREMENT = 1;
Rotary rotary = Rotary(ROTARY_PINA, ROTARY_PINB, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
Button2 button = Button2(ROTARY_PINC);
//-----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  delay(50);
  Serial.println("\n\nSimple Counter");

  rotary.setChangedHandler(rotate);
  rotary.setLeftRotationHandler(showDirection);
  rotary.setRightRotationHandler(showDirection);

  button.setClickHandler(click);
  button.setDoubleClickHandler(doubleclick);
}
//-----------------------------------------------------------------------------
void loop() {
  rotary.loop();
  button.loop();
}
//-----------------------------------------------------------------------------
void rotate(Rotary& r) {
  Serial.println(rotary.getPosition());
}
//-----------------------------------------------------------------------------
void showDirection(Rotary& r) {
  Serial.println(rotary.directionToString(rotary.getDirection()));
}
//-----------------------------------------------------------------------------
void doubleclick(Button2& btn) {
  Serial.println(__PRETTY_FUNCTION__);
}
//-----------------------------------------------------------------------------
void click(Button2& btn) {
  Serial.println(__PRETTY_FUNCTION__);
}
