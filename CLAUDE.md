# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Arduino firmware for the NEMUS haptic harpsichord: a USB-MIDI controller that reads QRE1113 IR reflectance sensors under each jack and sends Note On/Off. Target board is the **Arduino Nano 33 BLE** (mbed core); there are `#ifdef` stubs for ESP32/STM32 but they are not functional (e.g. `USBMIDI` is only provided by the Nano's `PluggableUSBMIDI.h`).

## Build / flash

Developed with Arduino IDE 2.3.x. Equivalent `arduino-cli` workflow:

```sh
# one-time deps
arduino-cli core install arduino:mbed_nano
arduino-cli lib install "Rotary" "Button2" "Adafruit FRAM SPI" "Adafruit NeoPixel"

# compile / upload the main firmware (each sketch is a directory)
arduino-cli compile --fqbn arduino:mbed_nano:nano33ble src/haptic_harpsichord
arduino-cli upload  --fqbn arduino:mbed_nano:nano33ble -p <port> src/haptic_harpsichord
arduino-cli monitor -p <port> -c baudrate=115200
```

There is no unit-test framework. `src/tests/*` are standalone hardware sketches (sensor, LED, rotary, FRAM, diagnostics) compiled and flashed individually the same way. `src/tests/fram_debug` is used to inspect/initialise FRAM contents.

Docs: Doxygen (`doxygen Doxyfile` → `docs/`), run on every push by `.github/workflows/gen_documentation.yml` and deployed to GitHub Pages. Keep the existing `/** @file ... @brief ... */` comment style.

## Repository layout

- `src/haptic_harpsichord/` — the production firmware (the only code that matters for most changes).
- `src/prototyping/` — historical sketches (3/6/7-note keyboards, data logger, ESP-IDF benchmark). Reference only; logic is similar but diverged. Don't update these when changing the main firmware.
- `src/tests/` — component diagnostic sketches.

## Architecture (`src/haptic_harpsichord/`)

Arduino concatenates all `.ino` files in the sketch folder into one translation unit (main `haptic_harpsichord.ino` first, then the rest alphabetically). So all globals, enums, pin constants, and FRAM address layout live in `haptic_harpsichord.ino`, and the other `.ino` files are just groups of free functions that use those globals directly. Forward declarations are only needed for things with default args or used in initialisers (see top of the main file).

**Sensor scan** (`sensor_functions.ino: readSensors`): 49 sensors = 7 PCBs (analog pins A0–A6) × 7 channels of a CD4051 mux (select pins D6–D8). Sensor index = `muxChannel + pcb * numMuxChannels`. Each scan swaps the `currSensorReadings`/`prevSensorReadings` pointers *first*, then fills `curr`. A per-sensor 8-sample ring-buffer moving average is applied, shifted by `AVG_SHIFT - 1`, so filtered readings are **13-bit (0–8191)** from a 12-bit ADC. Pressing a key makes the reading *drop*.

**Note detection**: edge-triggered threshold crossings between `prev` and `curr`. Two modes selected by `thresholdType`:
- `SINGLE_THRESHOLD` — one threshold per key (`singlePluckThresholds`).
- `HYSTERETIC` (default) — separate `pluckThresholds` (note on, falling below) and `releaseThresholds` (note off, rising above). Also gated on `jackStates[i]` (`RELEASED`↔`PLUCK`) so on/off strictly alternate. Only the active sensor range is scanned for notes.

**Key/note mapping**: only sensor indices `FIRST_ACTIVE_SENSOR..LAST_ACTIVE_SENSOR` (2–46) have keys; 0–1 and 47–48 are orphan positions with `0` in the note tables. `index2note()` (`midi_functions.ino`) looks up `frontRegisterNoteTable`/`backRegisterNoteTable` by `jackRegister` (tables are currently identical, E2=40 … C6=84). `key2index`/`index2key` convert between physical key number (1-based, reversed) and array index. Every `case` in `index2note` needs its `break`.

**Run modes**: `executeDebugMode` selects play mode (`hysteresisLoop`/`singleThresholdLoop`) or debug/calibration mode (`debugLoop`, `debug_functions.ino`); `loop()` dispatches on it and each mode's loop returns when it flips. Debug mode is entered by holding the encoder button at boot (`isPressedRaw()` — `isPressed()` only reflects the state at global construction) or `ALWAYS_DEBUG` (currently `false`). A long press (`MODE_SWITCH_HOLD_MS`, handler `longClickDetected`) toggles modes at runtime; leaving debug mode via `exitDebug()` saves thresholds to FRAM and resets the encoder to key select. Click/double-click handlers are no-ops outside debug mode. Debug mode also streams the selected key's reading + thresholds for the Serial Plotter and accepts serial commands (`p` toggles printing, `c` runs `autoCalibrate()`). Note detection itself lives only in `processHysteretic()` / `processSingleThreshold()` (`jack_state_functions.ino`), called by both modes — change it there, not in the loops.

**Calibration UI** (`rotary_functions.ino`): encoder click cycles `RotaryMode` (key select → pluck threshold → release threshold), rotation edits the value, double-click writes all thresholds to FRAM. LED colour indicates mode. `auto_calibration.ino` measures rest/noise then key travel and sets pluck at 25% / release at 75% of travel (RAM only until saved).

**Persistence** (`eeprom_functions.ino`): MB85RS64 SPI FRAM (software SPI on D2–D5). Layout is a sequence of 4-byte ASCII tags (`DATA`, `PLCK`, `RLSE`, `REGI`) each followed by its data block (`numSensors * 2` bytes for thresholds); addresses are computed in the main file. On read, a missing tag causes defaults to be written. Changing `numSensors` or the layout shifts all addresses and invalidates stored calibration.

**Errors**: `halt(code)` in `error_functions.ino` blinks the onboard RGB LED with an `error_codes.h` code and stops.

## Known inconsistencies to watch

Since readings moved to 13-bit ("New bit depth" commit), some code still assumes a 12-bit range: rotary threshold upper bound `4096`, FRAM default threshold `800`, and `printJackReading` plot ranges. Check these when touching thresholds or calibration. `setup()` also force-overrides `jackRegister = BACK_REGISTER` after reading FRAM.

## Hardware notes

Pinout is documented in `README.md` and the header of `haptic_harpsichord.ino`. If all sensors read noise, the mux INT/inhibit pin is likely floating — it must be grounded.
