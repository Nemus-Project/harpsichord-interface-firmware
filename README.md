# NEMUS Harpsichord Interface Firmware
[![DOI](https://zenodo.org/badge/964505533.svg)](https://doi.org/10.5281/zenodo.16848270)

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
| -------------: | :------- | --------: | :-------------- |
| EEPROM SPI CLK | D13      |       D12 | Rotary CLK      |
|     EEPROM VCC | 3.3v     |       D11 | Rotary DATA     |
|             NC | AREF     |       D10 | Rotary Switch   |
|          PCB 0 | A0       |        D9 | LED Data        |
|          PCB 1 | A1       |        D8 | Mux C           |
|          PCB 2 | A2       |        D7 | Mux B           |
|          PCB 3 | A3       |        D6 | Mux A           |
|          PCB 4 | A4       |        D5 | Mux Interrupt   |
|          PCB 5 | A5       |        D4 | EEPROM SPI MISO |
|          PCB 6 | A6       |        D3 | EEPROM SPI MOSI |
|             NC | A7       |        D2 | EEPROM SPI CS   |
|             NC | 5v       |       GND | GND             |
|             NC | RESET    |       RST | Reset Switch    |
|            GND | GND      |   RX / D1 | NC              |
|        PSU `+` | VIN      |   TX / D0 | NC              |

_NC_: Not Connected
