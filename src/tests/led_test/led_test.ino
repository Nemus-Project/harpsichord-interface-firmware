/**
 * @file led_test.ino
 * @brief LED Diagnostic
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
#include <Adafruit_NeoPixel.h>
//-----------------------------------------------------------------------------
// LED Variables
///
const size_t ledPin = 9;
///
Adafruit_NeoPixel leds(49, ledPin, NEO_GRB + NEO_KHZ800);
//-----------------------------------------------------------------------------

void setup() {
  leds.begin();
  leds.clear();
  leds.setBrightness(100);
  leds.fill(leds.Color(100, 90, 50), 0, 49);
  
  leds.show();
}

void loop() {
  rainbow(10);
}

void rainbow(int wait) {
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    leds.rainbow(firstPixelHue);
    leds.show();  // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}