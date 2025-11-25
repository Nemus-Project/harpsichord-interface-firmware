# NEMUS Harpsichord Interface Firmware
[![DOI](https://zenodo.org/badge/964505533.svg)](https://doi.org/10.5281/zenodo.16848270)

## Setup

First you will need the Arduino IDE. This project was compiled using [version 2.3.3](https://github.com/arduino/arduino-ide/releases/tag/2.3.3). Future revisions (`2.x.x`) should be fine, but behaviour may change for major revisions.

### Dependecies

The firware has the following library dependecies. 

- [Rotary](https://github.com/skathir38/Rotary)
- [Button2](https://github.com/LennartHennigs/Button2)
- [Adafruit NeoPixelRotary](https://github.com/adafruit/Adafruit_NeoPixel)
- [Adafruit FRAM SPI](https://github.com/adafruit/Adafruit_FRAM_SPI)

The MIDI functionality is currently provided through Arduino Nano 33 BLE mbed [`PluggableUSBMIDI.h`](https://github.com/arduino/ArduinoCore-mbed/blob/53bbb4b26e85fd165fe861ff3878a8d64029ca44/libraries/USBMIDI/PluggableUSBMIDI.h) header.

These dependencies can be installed manually or through the [`arduino-cli`](https://arduino.github.io/arduino-cli/).

If you are using [homebrew on macOS](https://brew.sh), in your Terminal type:

```sh
brew install arduino-cli
```

Otherwise, follow the [alternative install instructions](https://arduino.github.io/arduino-cli/1.3/installation/#use-the-install-script).

After installation, you can copy and paste the following into your terminal:

```sh
arduino-cli lib install "Rotary"
arduino-cli lib install "Button2"
arduino-cli lib install "Adafruit FRAM SPI"
arduino-cli lib install "Adafruit NeoPixel"
arduino-cli core install arduino:mbed_nano
```

Open the firmware and verify it works. Please [flag an issue](https://github.com/Nemus-Project/harpsichord-interface-firmware/issues/new/choose) if you come across any errors.

## Firmware

All firmware in `src/` directory which consists of

- `src/haptic_harpsichord/`: main firmware for the 49-key model
- `src/prototyping/`: firmware used through the prototyping process
- `src/tests/`: component functionaliy and diagnostics tests

@note the `prototyping/` directory contains source created during the prototyping process and is still largely unorganised. All functionality and logic is largely the same as the main `haptic_harpischord/` code, though there may be the odd hard-coded variable or subtle change in behaviour. As such it is kept here for reference.

### `haptic_harpsichord/`

This is the full firmware for the haptic harpsochord using the Arduino IDE.

Firmware was written around use of an Arduino Nano 33 BLE with some accomodation for an ESP32.

#### Components

- QRE1113
- 10 kΩ Potentiometer
- MB85RS64 SPI Non-Volatile FRAM 
- Rotary Encoder w/ tactile switch
- CD4051BE multiplexer
- WS2812B RGB LEDs

#### Wiring Table

|     Connection | Pin Left | Pin Right | Connection      |
| -------------: | -------: | :-------- | :-------------- |
| EEPROM SPI CLK |      D13 | D12       | Rotary CLK      |
|     EEPROM VCC |     3.3v | D11       | Rotary DATA     |
|             NC |     AREF | D10       | Rotary Switch   |
|          PCB 0 |       A0 | D9        | LED Data        |
|          PCB 1 |       A1 | D8        | Mux C           |
|          PCB 2 |       A2 | D7        | Mux B           |
|          PCB 3 |       A3 | D6        | Mux A           |
|          PCB 4 |       A4 | D5        | Mux Interrupt   |
|          PCB 5 |       A5 | D4        | EEPROM SPI MISO |
|          PCB 6 |       A6 | D3        | EEPROM SPI MOSI |
|             NC |       A7 | D2        | EEPROM SPI CS   |
|             NC |       5v | GND       | GND             |
|             NC |    RESET | RST       | Reset Switch    |
|            GND |      GND | RX / D1   | NC              |
|        PSU `+` |      VIN | TX / D0   | NC              |

_NC_: Not Connected
