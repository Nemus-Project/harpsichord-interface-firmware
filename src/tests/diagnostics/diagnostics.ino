/**
 * @file diagnostics.ino
 * @brief Component Diagnostics
 *
 * Confirm the connection of every component and print to serial any errors.
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
 */

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
