/*
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
