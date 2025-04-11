# NEMUS Harpsichord Interface Firmware

## Firmware

All firmware in `src/` directory which consists of

- `src/haptic_harpsichord/`: main firmware for the 49-key model
- `src/prototyping/`: firmware used through the prototyping process
- `src/tests/`: component functionaliy and diagnostics tests


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

†_NC_: Not Connected