# Jack sticker adjustment

**Status:** to do on all jacks. Found on 2026-10-21.

## Summary

Shifting the reflective sticker on a jack fixes the timing of Note On, so that it
matches the real pluck. Because this fixes it physically, no velocity-based
"pluck detection" is needed in the firmware for now. **The stickers on all jacks
need to be shifted in the same way.**

## The problem

Note On is sent when a key's reading crosses its pluck threshold. The reading
comes from the QRE1113 infrared sensor under the jack, looking at the sticker
on the jack. It therefore measures the jack's **position**, not the pluck
itself. The firmware has no way to know whether the plectrum has actually
passed the string at the moment the threshold is crossed.

To make Note On follow the real pluck, a velocity-based check was considered:
look for the change in the jack's speed when the plectrum releases the string,
around the threshold position. That turned out to be difficult:
- the deepest velocity dip happens **before** the pluck, during the free travel
  up to the string
- the jack then stalls against the string, and the pluck gives only a smaller
  second dip
- on very hard presses there is almost no stall, and the main dip comes right
  after the pluck

(See `CHANGELOG.md`, sections 9–15, for the velocity and recording tools built
for this.)

## The solution

**Physically shifting the sticker on the jack** solves it. Moving the sticker
moves its light gradient relative to the sensor. This changes which part of the
jack's travel the sensor's reading responds to, so the threshold crossing now
corresponds to the pluck. MOVE THE STICKER DOWNWARD

> **To fill in:**
> - which way the sticker is moved (towards the top / bottom of the jack):
>   _____
> - by how much (mm), and whether it is the same for every jack: _____
> - how to remove and reattach it without damaging it: _____
> - anything else that changed (sensor height, sticker type): _____

## Procedure for each jack

1. Shift the sticker as described above.
2. In calibration mode, open the live plot (`INSTRUCTIONS.md`, section 6.1) and
   select the key. Check that the reading still covers a clear range between
   rest and fully pressed.
3. Press the key slowly and check that Note On happens when the string is
   actually plucked.
   > **To fill in:** how this is checked (by ear, by watching the jack): _____
4. Tick the jack in the checklist below.

## After all jacks are done

The readings change when a sticker moves, so **the stored calibration is no
longer valid**. Recalibrate the whole keyboard (`INSTRUCTIONS.md`, section 5):
enter calibration mode, send `c`, play every key, and save.

Then check every key once more with soft, normal and hard presses.

## Checklist

The jacks are listed by note. The key numbers are the ones shown in the serial
messages and the plot (`K47` etc.), which depend on the register setting.

| Note | MIDI | Key no. (back register) | Key no. (front register) | Sticker shifted | Recalibrated | Checked |
|---|---|---|---|---|---|---|
| E2 | 40 | 47 | 3 | ☐ | ☐ | ☐ |
| F2 | 41 | 46 | 4 | ☐ | ☐ | ☐ |
| F#2 | 42 | 45 | 5 | ☐ | ☐ | ☐ |
| G2 | 43 | 44 | 6 | ☐ | ☐ | ☐ |
| G#2 | 44 | 43 | 7 | ☐ | ☐ | ☐ |
| A2 | 45 | 42 | 8 | ☐ | ☐ | ☐ |
| Bb2 | 46 | 41 | 9 | ☐ | ☐ | ☐ |
| B2 | 47 | 40 | 10 | ☐ | ☐ | ☐ |
| C3 | 48 | 39 | 11 | ☐ | ☐ | ☐ |
| C#3 | 49 | 38 | 12 | ☐ | ☐ | ☐ |
| D3 | 50 | 37 | 13 | ☐ | ☐ | ☐ |
| D#3 | 51 | 36 | 14 | ☐ | ☐ | ☐ |
| E3 | 52 | 35 | 15 | ☐ | ☐ | ☐ |
| F3 | 53 | 34 | 16 | ☐ | ☐ | ☐ |
| F#3 | 54 | 33 | 17 | ☐ | ☐ | ☐ |
| G3 | 55 | 32 | 18 | ☐ | ☐ | ☐ |
| G#3 | 56 | 31 | 19 | ☐ | ☐ | ☐ |
| A3 | 57 | 30 | 20 | ☐ | ☐ | ☐ |
| Bb3 | 58 | 29 | 21 | ☐ | ☐ | ☐ |
| B3 | 59 | 28 | 22 | ☐ | ☐ | ☐ |
| C4 | 60 | 27 | 23 | ☐ | ☐ | ☐ |
| C#4 | 61 | 26 | 24 | ☐ | ☐ | ☐ |
| D4 | 62 | 25 | 25 | ☐ | ☐ | ☐ |
| D#4 | 63 | 24 | 26 | ☐ | ☐ | ☐ |
| E4 | 64 | 23 | 27 | ☐ | ☐ | ☐ |
| F4 | 65 | 22 | 28 | ☐ | ☐ | ☐ |
| F#4 | 66 | 21 | 29 | ☐ | ☐ | ☐ |
| G4 | 67 | 20 | 30 | ☐ | ☐ | ☐ |
| G#4 | 68 | 19 | 31 | ☐ | ☐ | ☐ |
| A4 | 69 | 18 | 32 | ☐ | ☐ | ☐ |
| Bb4 | 70 | 17 | 33 | ☐ | ☐ | ☐ |
| B4 | 71 | 16 | 34 | ☐ | ☐ | ☐ |
| C5 | 72 | 15 | 35 | ☐ | ☐ | ☐ |
| C#5 | 73 | 14 | 36 | ☐ | ☐ | ☐ |
| D5 | 74 | 13 | 37 | ☐ | ☐ | ☐ |
| D#5 | 75 | 12 | 38 | ☐ | ☐ | ☐ |
| E5 | 76 | 11 | 39 | ☐ | ☐ | ☐ |
| F5 | 77 | 10 | 40 | ☐ | ☐ | ☐ |
| F#5 | 78 | 9 | 41 | ☐ | ☐ | ☐ |
| G5 | 79 | 8 | 42 | ☐ | ☐ | ☐ |
| G#5 | 80 | 7 | 43 | ☐ | ☐ | ☐ |
| A5 | 81 | 6 | 44 | ☐ | ☐ | ☐ |
| Bb5 | 82 | 5 | 45 | ☐ | ☐ | ☐ |
| B5 | 83 | 4 | 46 | ☐ | ☐ | ☐ |
| C6 | 84 | 3 | 47 | ☐ | ☐ | ☐ |
