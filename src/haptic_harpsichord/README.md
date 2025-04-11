# Haptic Harpsichord Firmware

This documentation contain information for developing and deploying the firmware for the Haptic Harpsichord MIDI Controller

## Wiring

Pin layout adheres to the Arduino nano form factor
  
| Connection      | Pin   |
| --------------- | ----- |
| EEPROM SPI CLK  | D13   |
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
| Mux Interrupt   | D5      |
| EEPROM SPI MISO | D4      |
| EEPROM SPI MOSI | D3      |
| EEPROM SPI CS   | D2      |
| NC              | RX / D1 |
| NC              | TX / D0 |

## MIDI

## Power

The Arduino and the circuit are to be powered by 5V 5A PSU.

## Troubleshoooting

### Sensor Data Noise

If the sensors seem to return random noise and do not respond to an IR LED it is possible the interrupt for the multiplexers is not connected or is at a positive voltage. Ground the INT pin of the multiplexer.
