# Haptic Harpsichord — User Instructions

How to operate the keyboard from the outside: rotary encoder, USB serial and
LEDs. This describes the firmware in `src/haptic_harpsichord/` on the
`lab-keyboard` branch. See `CHANGELOG.md` for how it differs from `MayConcerts`.

---

## 1. Overview

The keyboard has two modes:

| Mode | What it does | Onboard blue LED |
|---|---|---|
| **Play** (default) | Reads the sensors and sends MIDI. Encoder clicks and rotation are ignored. | off |
| **Calibration / Debug** | Still sends MIDI, and also lets you set thresholds with the encoder, stream sensor data to the Serial Plotter, and run auto-calibration. | on |

**MIDI**
- Sent as a class-compliant USB MIDI device: no driver, it appears as a MIDI
  input on the computer.
- Channel 1, fixed velocity 100.
- 45 keys, chromatic from **E2 (MIDI 40)** to **C6 (MIDI 84)**.

**Serial**
- Uses the same USB cable.
- Open the Arduino IDE's **Serial Monitor** or **Serial Plotter** at **115200** baud.
- Commands are single characters, typed into the IDE's message box and sent.

**Jack register.** The sensor rail reads the keys in opposite order under the
front and the back register, so the firmware has two mirrored note mappings.
The register is saved in FRAM and can be changed from serial (section 6).

**Key numbers.** Serial messages refer to sensor positions by number (49 down
to 1). Positions 1, 2, 48 and 49 are unused, with no key above them.

| Register | Key 47 | Key 3 |
|---|---|---|
| **Back** | E2 (lowest) | C6 (highest) |
| **Front** | C6 (highest) | E2 (lowest) |

---

## 2. Startup

1. Power the board (USB or PSU). Startup takes about 3 seconds.
2. The firmware loads the thresholds saved in the FRAM memory chip.
3. It prints a summary on serial:
   ```
   Firmware Version: 0.2.2
   Threshold Type: Hysteretic
   Jack Register: Back
   Mode: Standard        ← "DEBUG" if it started in calibration mode
   ```
4. It starts in **Play** mode, unless the encoder button was held (see 3.1).

**First start with an empty or new FRAM chip.** The firmware fills the memory
with default thresholds (1600) and selects the **back** register, printing:
```
No jack register stored, using Back. Send f / b in debug mode to change.
```
Startup continues without waiting for input. If the sensor rail is under the
front register, switch with `f` in calibration mode (section 6). Then
**calibrate** (section 5): the default thresholds will not play correctly.

---

## 3. Switching modes

### 3.1 At startup

Hold the encoder button at any time during the ~3 s startup. Calibration mode
starts when you **release** it, and the blue LED turns on.

### 3.2 While running

**Hold the encoder button for 2 seconds.** The switch happens as soon as the
2 seconds pass; you don't have to release first. This works in both directions:

- **Play → Calibration:** the blue LED turns on and the selected key lights up
  on the LED strip.
- **Calibration → Play:** **all thresholds are saved to FRAM**, the blue LED and
  strip turn off, and the encoder returns to key select.

> Holding for 2 s always switches mode. A press shorter than 2 s is a normal
> click (only active in calibration mode).

---

## 4. Manual calibration (encoder)

Only in calibration mode. Each key has two thresholds:

- **Pluck:** when the reading falls below it, a Note On is sent.
- **Release:** when the reading rises back above it, a Note Off is sent.

Release must be **above** pluck. Pressing a key makes its reading go **down**.

### Controls

| Action | Effect |
|---|---|
| **Turn** | Depends on the current encoder mode (table below). |
| **Click** | Next encoder mode: Key select → Pluck → Release → Key select … |
| **Double-click** | Save all thresholds to FRAM. The whole strip flashes blue briefly. Stays in calibration mode. |
| **Hold 2 s** | Save and go back to Play mode (section 3.2). |

| Encoder mode | Strip LED colour | Turning does |
|---|---|---|
| **Key select** | blue | Selects a key, one per step. Starts on key 47 (E2 on the back register, C6 on the front). |
| **Pluck threshold** | green | Changes the selected key's pluck threshold in steps of 10. |
| **Release threshold** | purple | Changes the selected key's release threshold in steps of 10. |

Threshold values range from 0 to 8190.

Changes take effect **immediately** while you play, but are only kept in memory
until saved. To **discard** unsaved changes, press the board's **reset button**
instead of saving.

### Workflow

1. Enter calibration mode (section 3).
2. Open the **Serial Plotter** (section 6) to see the selected key's reading and
   thresholds.
3. In key select (blue), turn to the key.
4. Click → pluck (green), and turn until the green line is where you want the
   note to start.
5. Click → release (purple), and turn until the release line sits above pluck,
   where you want the note to stop.
6. Click → back to key select, and choose the next key.
7. Double-click to save, or hold 2 s to save and return to Play mode.

---

## 5. Auto-calibration

Sets pluck and release for every key automatically from how far each key
actually travels. Recommended after any change to the sensors, the mechanics or
the firmware's reading scale.

1. Enter calibration mode (section 3). Threshold type must be **Hysteretic**,
   which is the default; check the startup summary.
2. Open the **Serial Monitor** at 115200 baud.
3. *(Optional)* Send `p` to pause the plotter stream so the messages are easy
   to read.
4. Send **`c`**, then follow the three phases:

| Phase | Serial message | Strip LEDs | What to do |
|---|---|---|---|
| **1. Rest** | `[AUTOCAL] 1/3 RESTING — hands off keyboard` | magenta | **Don't touch the keyboard.** It measures each key's resting level and noise (~1–2 s). |
| **2. Play** | `[AUTOCAL] 2/3 PLAY — press each key a few times` | dim green; a key turns **bright green** once a press has been detected | Press **every** key fully a few times. Lasts up to **30 s**; send any character to finish early. |
| **3. Compute** | `[AUTOCAL] done — N/49 keys calibrated` | **yellow** = key skipped | Nothing. Skipped keys keep their previous thresholds, and each is listed as `key N not pressed — kept existing values`. |

5. **Save.** Double-click, or hold 2 s to save and return to Play mode. Results
   are lost on power-off if you don't save.

**What it calculates.** For each key, pluck is set at 25% and release at 75% of
the travel from fully pressed to rest. A minimum gap is kept between the two, so
noise can't retrigger a note.

**Expected result: 45/49.** The four unused positions (keys 1, 2, 48, 49) are
always reported "not pressed". Any other skipped key was not pressed deeply
enough: run it again and press that key fully, or set it by hand (section 4).

You can fine-tune individual keys by hand afterwards.

---

## 6. Serial Plotter

Only in calibration mode. It streams the **selected key** continuously. It is
on by default whenever you enter calibration mode.

Open **Tools → Serial Plotter** in the Arduino IDE, at 115200 baud.

| Line | Meaning |
|---|---|
| `K<n>` | live sensor reading of key *n* (drops when pressed) |
| `P<n>` | pluck threshold |
| `R<n>` | release threshold |
| `m` / `M` | lower / upper edge of the plot window |

The window follows the selected key and its thresholds. It shows the key's
travel from fully pressed (bottom) to rest (top), and rescales when you change
key or edit a threshold.

A healthy key looks like this:
- at rest, `K` sits above `R`
- when pressed, `K` falls below `P`
- when released, `K` rises back above `R`

### Velocity view

Send `v` to switch the plotter to the selected key's **velocity**, and `v`
again to switch back. It starts in the position view.

| Line | Meaning |
|---|---|
| `V<n>` | velocity of key *n*, in reading units per second |
| `L<n>` | lowest velocity of key *n* since its last Note Off |
| `Z` | zero line |
| `m` / `M` | fixed scale at −/+ 15,000 units per second |

- **Below zero:** the key is going **down**.
- **Above zero:** the key is coming **up**.
- **At rest:** the trace sits near zero, with small noise.

**Pluck peak vs. noise (`L`).** `L` is a running minimum: it only ever steps
down, and resets to 0 at each Note Off. To check whether the pluck can be told
apart from noise:

1. Release the key (Note Off): `L` resets to 0.
2. Leave it at rest for a moment: `L` settles at the **noise floor**, the lowest
   value the noise reaches.
3. Press the key: at the pluck, `L` steps down to the **pluck peak**.
4. Release: `L` resets for the next try.

The larger the gap between the noise floor and the pluck peak, the more reliably
the peak can be detected. Try soft and slow presses too, since they give the
smallest peaks.

The scale is fixed, so noise at rest appears at its real size compared to a key
press. A faster movement extends the plot beyond the scale. The range is set by
`VELOCITY_PLOT_RANGE` in the code. The velocity is the slope of a straight line
fitted through the last 5 samples, so it lags the movement slightly (about 2
scans). The number of samples is set by `VELOCITY_WINDOW` in the code (2–32):
fewer samples react faster but are noisier, more samples are smoother but react
later. It does not affect the readings or note detection.

### Serial commands (calibration mode)

| Send | Effect |
|---|---|
| `p` | Toggle the plotter stream on/off. |
| `c` | Run auto-calibration (section 5). |
| `v` | Switch the plotter between the position view and the velocity view (below). |
| `f` | Switch to the **front** register and save it to FRAM. |
| `b` | Switch to the **back** register and save it to FRAM. |

`f` / `b` print the active register (`Jack Register: Front` / `Back`). Any note
sounding at that moment is released first, so nothing hangs. Thresholds belong
to sensor positions, so they are kept. If the rail is physically moved to the
other register, recalibrate, because the jacks sit differently.

Other characters are ignored. In Play mode, serial input is ignored entirely.

---

## 7. LEDs at a glance

**LED strip (one LED per key)**

| Colour | Meaning |
|---|---|
| blue (one key) | calibration: key select, selected key |
| green (one key) | calibration: editing pluck |
| purple (one key) | calibration: editing release |
| all blue, brief flash | thresholds saved (double-click) |
| all magenta | auto-cal phase 1, don't touch |
| dim / bright green | auto-cal phase 2: waiting / key detected |
| yellow | auto-cal: key skipped |
| off | Play mode |

**Onboard LEDs (on the Arduino)**

| LED | Meaning |
|---|---|
| RGB **blue** on | calibration mode |
| RGB **red** on | saving to FRAM failed |
| built-in LED blinking **3 times, then a pause**, repeating | FRAM chip not found: the firmware has stopped. Check the FRAM wiring and reset. |

---

## 8. Inspecting or resetting the FRAM (`fram_debug` sketch)

`src/tests/fram_debug/` is a **separate firmware** for looking at or overwriting
what is stored in the FRAM. Flashing it replaces the keyboard firmware until you
flash `haptic_harpsichord` again.

Serial Monitor at **9600** baud. Commands:

| Send | Effect |
|---|---|
| `a` | print stored values |
| `m` | memory dump (ASCII) |
| `s` | set **all** single thresholds to one value |
| `p` | set **all** pluck thresholds to one value |
| `r` | set **all** release thresholds to one value |
| `j` | set jack register (1 front, 2 back); prefer `f` / `b` in calibration mode |

Caveats:
- This sketch still assumes the old 12-bit scale. It only accepts values 0–4095,
  while the keyboard firmware now uses 0–8190.
- `s` / `p` / `r` overwrite **every** key's calibration with a single value.
- `a` prints the first 7 values repeated for each board instead of all 49.

To check calibration values, prefer the Serial Plotter in calibration mode.

---

## 9. Not adjustable without changing the code

These are set in the firmware source (`src/haptic_harpsichord/haptic_harpsichord.ino`)
and need a re-flash:

- Threshold type (`thresholdType`): must stay **hysteretic**. Single threshold
  is deprecated and must not be used; the boot summary prints a warning if it is
  selected.
- Note mapping / range (`frontRegisterNoteTable`, `backRegisterNoteTable`).
- MIDI channel and velocity.
- Mode-switch hold time (`MODE_SWITCH_HOLD_MS`, 2 s).
- Always starting in calibration mode (`ALWAYS_DEBUG`).
