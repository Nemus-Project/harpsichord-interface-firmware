/*
  Haptic Harpsichord: LED Test

  Testing functionality of LEDs in a 49-key strip for the haptic harpsichord.

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
// Board Agnostics Libraries
#include <Rotary.h>
#include <Button2.h>
#include <Adafruit_NeoPixel.h>

const int numSensors = 49;
const size_t ledPin = 9;
Adafruit_NeoPixel leds(numSensors, ledPin, NEO_GRB + NEO_KHZ800);

void setup() {  
  leds.begin();
  leds.clear();  
  leds.show();
  leds.fill(leds.Color(100,0,0),0,numSensors);
  leds.setBrightness(25); // Set BRIGHTNESS to about 1/5 (max = 255)
  leds.setPixelColor(30,leds.Color(0,100,0));
  leds.show();
}

void loop() {
  // // Fill along the length of the strip in various colors...
  // colorWipe(leds.Color(255,   0,   0), 500); // Red
  // colorWipe(leds.Color(  0, 255,   0), 500); // Green
  // colorWipe(leds.Color(  0,   0, 255), 500); // Blue

  // // Do a theater marquee effect in various colors...
  // theaterChase(leds.Color(127, 127, 127), 50); // White, half brightness
  // theaterChase(leds.Color(127,   0,   0), 50); // Red, half brightness
  // theaterChase(leds.Color(  0,   0, 127), 50); // Blue, half brightness

  rainbow(10);             // Flowing rainbow cycle along the whole strip
  // theaterChaseRainbow(500); // Rainbow-enhanced theaterChase variant
}


// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// leds.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<leds.numPixels(); i++) { // For each pixel in leds...
    leds.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    leds.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la leds.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      leds.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<leds.numPixels(); c += 3) {
        leds.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      leds.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole leds. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    // leds.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    leds.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // leds.rainbow(firstPixelHue, 1, 255, 255, true);
    leds.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      leds.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<leds.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (leds.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / leds.numPixels();
        uint32_t color = leds.gamma32(leds.ColorHSV(hue)); // hue -> RGB
        leds.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      leds.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
