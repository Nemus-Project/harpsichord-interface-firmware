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
The register is saved in FRAM and can be changed from serial (section 6.5).

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
front register, switch with `f` in calibration mode (section 6.5). Then
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
2. Open the **live plot** (section 6) to see the selected key's reading and
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

## 6. Live plot (calibration mode)

In calibration mode the firmware continuously streams the **selected key** over
USB serial. It is on by default whenever you enter calibration mode. There are
two ways to view it:

- **6.1 The Python plotter (recommended):** `tools/press_capture.py monitor`.
  Smoother, readable at any speed, keyboard shortcuts, and it can record presses.
- **6.2 The Arduino IDE Serial Plotter:** no setup, but more limited.

Only **one** program can use the board's serial port at a time: close one
before opening the other.

### 6.1 Python plotter (recommended)

**One-time setup**

You need Python 3 with `numpy` and `matplotlib` (already included in Anaconda),
plus `pyserial`. In a Terminal, run:

```sh
pip install pyserial
```

**Each session**

1. **Close** the Arduino IDE Serial Monitor and Serial Plotter, if open.
2. On the keyboard, enter calibration mode (section 3) and select a key.
3. Open a Terminal and go to the repository folder:
   ```sh
   cd ~/Desktop/LocalStuff/GitHub/harpsichord-interface-firmware
   ```
4. Find the board's port name:
   ```sh
   ls /dev/cu.usbmodem*
   ```
   It prints something like `/dev/cu.usbmodem1101`. If it says "no matches
   found", the board is not connected, or another program is using it.
5. Start the plotter with **that** port name:
   ```sh
   python3 tools/press_capture.py monitor --port /dev/cu.usbmodem1101
   ```
6. A window titled **"press_capture monitor - live"** opens. **Click on it**:
   the keys below only work while this window is in front, not in the Terminal.

> Type the commands one at a time, exactly as shown. Don't paste comments after
> them: in the macOS Terminal, text after `#` is not ignored, and an apostrophe
> leaves the Terminal waiting at a `quote>` prompt. If that happens, press
> **Ctrl+C** and start again.

**Keys (in the plot window)**

| Key | Effect |
|---|---|
| `v` | switch between the **position** view and the **velocity** view |
| `p` | live stream on/off |
| `r` | record one press (section 6.4) |
| `l` | next recording label |
| `q` | quit (closing the window works too) |

The encoder works as usual while the plotter is running. Selecting another key
or editing a threshold updates the plot straight away.

**Title line**

The top of the window shows the current label, the number of saved recordings,
and the status:

| Status | Meaning |
|---|---|
| `live` | data is arriving |
| `no data - is the board in calibration mode?` | nothing received for 2 s: the board is in Play mode, or the stream is off (`p`) |
| `ARMED - press and release the key` | a recording is waiting for the press |
| `receiving capture...` | a recording is being transferred |
| `SERIAL ERROR: …` | the board was unplugged or the port was lost: quit and restart |

If the stream is off when the plotter starts, it turns it on by itself.

**Options**

- `--points 3000`: show more history (default 1500 samples)
- `--labels a,b,c`: your own recording labels
- `--out-dir folder`: where recordings are saved (default `captures/`)

### 6.2 Arduino IDE Serial Plotter

Open **Tools → Serial Plotter** in the Arduino IDE, at 115200 baud. Commands
(section 6.5) are typed in its message box and sent. It cannot save recordings.

### 6.3 What the lines mean

**Position view** (default):

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

**Velocity view** (`v`):

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
3. Press the key: `L` steps down to the **deepest point of the press**. On
   normal presses this is the free travel just before the plectrum meets the
   string, not the pluck itself.
4. Release: `L` resets for the next try.

The scale is fixed, so noise at rest appears at its real size compared to a key
press. A faster movement extends the plot beyond the scale. The range is set by
`VELOCITY_PLOT_RANGE` in the code. The velocity is the slope of a straight line
fitted through the last 5 samples, so it lags the movement slightly (about 2
scans). The number of samples is set by `VELOCITY_WINDOW` in the code (2–32):
fewer samples react faster but are noisier, more samples are smoother but react
later. It does not affect the readings or note detection.

### 6.4 Recording a press (`r`)

Records one press of the selected key, scan by scan, for offline analysis
(for example, studying the velocity around the pluck). Not needed for normal
use.

**With the Python plotter (6.1):** press `r` in the plot window. When the title
shows **ARMED**, press and release the key. The recording is **saved
automatically** in `captures/` (named with date, key and current label) and
shown in a second window. Press `r` again for the next one; `l` changes the label
(soft, normal, hard, very-hard, no-pluck, barely-pluck). `captures/` is in
`.gitignore`.

**What the firmware does:**
1. On `r`, the key's LED turns **white** and the plot stream pauses. Nothing is
   printed while recording, so the timing is the same as in normal playing.
2. Recording **starts** when the reading falls below the release threshold, and
   keeps the ~500 scans before that (key at rest, start of the movement).
3. It **ends** once the key has been back above the release threshold for 300
   scans, or after 4000 scans (~4 s). `r` again cancels.
4. The press is sent as CSV between `[CAPTURE] BEGIN` and `[CAPTURE] END`.
   Columns: `t_us` (time in µs), `raw` (raw sample), `reading` (filtered
   reading), `note_on` (1 while the note is on). Header lines starting with `#`
   give the key, its thresholds and settings.

The Arduino **Serial Plotter cannot save** recordings: the data is lost. The
Serial Monitor shows it, but it has to be copied into a file by hand.

**Looking at saved recordings** (no board needed):

```sh
python3 tools/press_capture.py plot captures/FILE.csv
```

```sh
python3 tools/press_capture.py plot captures/*soft*.csv --overlay
```

Add `--window 9` to recompute the velocity with another window. The plots show
position over time, velocity over time, and velocity against position (where an
event at a fixed position lines up vertically across presses).

A fixed number of presses can also be recorded from the Terminal, without the
live plot:

```sh
python3 tools/press_capture.py capture --port /dev/cu.usbmodem1101 --label soft --count 10
```

### 6.5 Serial commands (calibration mode)

In the Python plotter, `v`, `p` and `r` are sent for you. In the Arduino IDE,
type them in the Serial Monitor / Plotter message box.

| Send | Effect |
|---|---|
| `p` | Toggle the plot stream on/off. |
| `v` | Switch between the position view and the velocity view. |
| `c` | Run auto-calibration (section 5). |
| `f` | Switch to the **front** register and save it to FRAM. |
| `b` | Switch to the **back** register and save it to FRAM. |
| `r` | Record one press of the selected key (6.4). Send again to cancel. |

`f` / `b` print the active register (`Jack Register: Front` / `Back`). Any note
sounding at that moment is released first, so nothing hangs. Thresholds belong
to sensor positions, so they are kept. If the rail is physically moved to the
other register, recalibrate, because the jacks sit differently.

`c`, `f` and `b` are not available as keys in the Python plotter. To use them,
quit the plotter and use the Arduino IDE Serial Monitor.

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
| white (one key) | press recording armed (`r`) |
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
