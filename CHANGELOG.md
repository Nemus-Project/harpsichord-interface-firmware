# Changelog — `lab-keyboard`

Changes on the `lab-keyboard` branch relative to `MayConcerts`, which is kept
untouched as the reference version used for the May 2026 concerts.

All paths are in `src/haptic_harpsichord/` unless stated otherwise.

---

## 1. Single note-detection logic for play and debug modes

*Commit `846d241` — "uniformed process loop"*

**Problem.** Note detection was written twice: once in the play loops
(`hysteresisLoop`, `singleThresholdLoop` in `haptic_harpsichord.ino`) and once
in the debug loops (`debug_functions.ino`). The two copies had diverged. The
play version was an older one without the `jackStates` gate, while the debug
version was the correct one. Because `ALWAYS_DEBUG` was `true`, the debug loop
was what actually ran on stage. The debug loop also scanned all 49 sensors,
including the four orphan positions, which could emit MIDI note 0.

**Change.**
- New `processHysteretic()` and `processSingleThreshold()` in
  `jack_state_functions.ino`, copied from the debug-loop logic (pluck below the
  threshold → Note On, rise above release → Note Off, gated by `jackStates` so
  on/off strictly alternate).
- Both play loops and both debug loops now call these functions. Note logic
  lives in one place only.
- Detection is limited to the active sensors `FIRST_ACTIVE_SENSOR..LAST_ACTIVE_SENSOR`
  (2–46). Orphan sensors 0–1 and 47–48 no longer send MIDI.

## 2. Play mode by default, encoder long press to switch modes

*Commit `580c303` — "add enter debug mode with 2-seconds rotary press"*

**Problem.** With `ALWAYS_DEBUG` set to `true`, the firmware always ran the debug
loop, including serial plotter output on every scan, and there was no way to
leave it. The "hold the button at boot" check used `button.isPressed()`, which
only reports the state captured when the global `button` object is constructed
at power-on, not the state at the end of `setup()`.

**Change.**
- `ALWAYS_DEBUG` is now `false`: the board boots into play mode.
- **Entering calibration at boot:** hold the encoder button at any time during
  the ~3 s startup (`button.isPressedRaw()` reads the pin directly). Calibration
  starts on release. After the release, `button.begin()` resyncs Button2 so the
  boot press is not mistaken for a long press.
- **Switching while running:** hold the encoder button for 2 s
  (`MODE_SWITCH_HOLD_MS`) to toggle between play and calibration
  (`longClickDetected()` in `rotary_functions.ino`). `loop()` dispatches on
  `executeDebugMode`, and each mode's loop returns when the flag changes.
- Leaving calibration (`exitDebug()`) saves thresholds to FRAM, turns off the
  onboard blue LED (it was written `LOW`, i.e. on), refreshes the LED strip
  (`leds.show()` was missing), and resets the encoder to key select.
- Click and double-click are ignored in play mode, so a stray press during a
  performance cannot change LEDs or write FRAM.
- The "Mode:" line printed at boot reflects the actual starting mode.
- Side effect: Button2's long-click time also decides what counts as a click.
  It used to be the library default of 200 ms, so presses longer than 0.2 s were
  silently ignored. Now presses up to 2 s count as clicks.

## 3. Encoder key-select range and documentation workflow

*Commit `748b148` — "fixed sensors bounds"*

**Encoder bound.** In `click()`, key-select mode set `rotary.setUpperBound(numSensors)`.
Rotary bounds are inclusive, so the encoder could reach index 49, one past the
end of the 49-element threshold arrays. Editing a threshold there wrote past
`pluckThresholds` / `releaseThresholds` and could save corrupted values to FRAM.
It now uses `MAX_POS` (`numSensors - 1`), `MIN_POS` and `INCREMENT`, matching
the encoder's constructor and `exitDebug()`.

**Doxygen workflow** (`.github/workflows/gen_documentation.yml`). The workflow ran
on every push to every branch, but only `main` is allowed to deploy to the
protected `github-pages` environment. Pushes to other branches therefore always
failed. It now runs only on pushes to `main`.

## 4. 13-bit sensor scale, and a plot window that follows the selected key

*Commit `e643b2e` — "fix bit depth issues"*

**Problem.** See [Background: 12-bit vs 13-bit readings](#background-12-bit-vs-13-bit-readings).
Several values in the code still assumed the old 12-bit range.

**Change.** New constants in `haptic_harpsichord.ino`, next to the filter settings:

| Constant | Value | Meaning |
|---|---|---|
| `ADC_MAX` | 4095 | largest raw 12-bit ADC value |
| `SENSOR_READING_MAX` | 8190 | largest filtered reading, derived from `AVG_SIZE` / `AVG_SHIFT` |
| `DEFAULT_THRESHOLD` | 1600 | threshold written to empty FRAM |
| `VELOCITY_DELTA_MAX` | 360 | reading change that maps to full velocity |

| Where | Before | After |
|---|---|---|
| Encoder threshold limit (`rotary_functions.ino`, 3 modes) | 4096 | `SENSOR_READING_MAX` |
| FRAM defaults (`eeprom_functions.ino`) | 800 | `DEFAULT_THRESHOLD` (1600) |
| Velocity mapping (`sensor_functions.ino`, not used by the play loops yet) | 180 | `VELOCITY_DELTA_MAX` (360) |
| LED brightness mapping (`led_functions.ino`, not called at present) | 4096 | `SENSOR_READING_MAX` |
| Serial Plotter window (`debug_functions.ino`) | fixed 0–4098 / 1100–1300 | follows the selected key |

**Plot window.** The new `printJackReadingAroundThresholds()` in
`print_functions.ino` sets the plotter's `m`/`M` limits from the selected key's
thresholds:

```
M  release + gap     (≈ key at rest)
   release
     ← gap →
   pluck
m  pluck − gap       (≈ key fully pressed)
```

- The margin above and below the thresholds equals the pluck–release gap, with a
  minimum of 100.
- For an auto-calibrated key, where pluck sits at 25% and release at 75% of its
  travel, the window shows the whole key movement.
- The window rescales when you select another key or edit a threshold.
- In single-threshold mode, the window is the threshold ± 100.
- Limits are clamped to 0–8190, and a key with pluck above release is handled.

**Action required.** Thresholds stored in FRAM before `76a77f7` are not
converted. Recalibrate once:

1. Hold the encoder button for 2 s to enter calibration mode.
2. Send `c` over serial.
3. Play every key.
4. Hold the button for 2 s again to leave calibration and save to FRAM (or
   double-click to save without leaving).

## 5. Front / back register with mirrored note mapping

**Background.** The sensor rail reads the keys in opposite order depending on
which jack register it sits under. Matthew's original design (`086ae04`, on
`main`) had two mirrored tables: front descending, back ascending. It never
worked, though: `index2note()` had no `break`s, so the front table was always
used. On `MayConcerts`, `76a77f7` added the `break`s, but made both tables
identical (ascending) and forced `BACK_REGISTER` in `setup()`.

**Change.**
- `frontRegisterNoteTable` is the mirror of the back table again:
  index 2 = C6 … index 46 = E2. The back table is unchanged: index 2 = E2 …
  index 46 = C6, which is confirmed correct for the back register on this
  keyboard. Both keep the orphan positions 0–1 and 47–48. These are symmetric,
  so the active range 2–46 is the same for both registers.
- `setup()` no longer forces `BACK_REGISTER`. The register stored in FRAM is used.
- New serial commands in (hysteretic) debug mode: `f` = front, `b` = back
  (`changeJackRegister()` in `haptic_harpsichord.ino`). Notes currently
  sounding are released with the old mapping first, so none are left hanging.
  The new register is saved to FRAM immediately.
- New `writeRegisterToEEPROM()` in `eeprom_functions.ino`. It sets FRAM
  write-enable before **each** write: the chip clears write-enable after every
  write, and `fram.write()` does not set it. The first-boot register path used
  to write the `REGI` tag without write enable, so on a fresh chip the tag was
  not saved and the prompt came back on every boot. That path now uses this
  function.

## 6. FRAM threshold tags saved correctly on a fresh chip

**Problem.** The same missing write-enable (see section 5) affected the
threshold tags (`DATA`, `PLCK`, `RLSE`) in `readPluckFromEEPROM()`. Each tag was
written before `fram.writeEnable(true)`, so on a fresh FRAM chip the tags were
never saved. On every boot, the firmware then found no tag and overwrote the
saved thresholds with the defaults: calibration was lost at each power cycle.
Chips whose tags had already been written, for example by `fram_debug`, were
not affected.

**Change.** `fram.writeEnable(true)` is now called before each tag write, and
`readPluckFromEEPROM()` ends with `fram.writeEnable(false)`.

## 7. Single-threshold mode marked deprecated

Single-threshold detection (`SINGLE_THRESHOLD`) must not be used. It was not
removed, but it is now clearly marked:
- `@warning` / `@deprecated` Doxygen comments on the `ThresholdType` enum,
  `singleThresholdLoop()`, `singleThresholdDebugLoop()` and
  `processSingleThreshold()`, which also appear in the generated docs'
  "Deprecated" list.
- A comment on `thresholdType` saying it must stay `HYSTERETIC`.
- A serial warning in the boot summary if `thresholdType` is ever set to
  `SINGLE_THRESHOLD`.

*Sections 5–7: commit `48fdabf` — "add double register, fix two FRAM bugs,
deprecated single threshold"*

## 8. No blocking register prompt on a fresh FRAM chip

**Problem.** With no register stored in FRAM, `setup()` stopped in
`setRegister()` and waited forever for `1` / `2` over serial. Without a
computer attached, the keyboard never started and gave no visible reason. The
encoder could change the choice, but a click could not confirm it: the wait
loop never called `button.loop()`, and `click()` ignores clicks outside debug
mode.

**Change.** `readPluckFromEEPROM()` now defaults to `BACK_REGISTER`, prints
`No jack register stored, using Back. Send f / b in debug mode to change.`,
saves it to FRAM and continues booting. `setRegister()` is no longer called.
It is kept in the code, marked `@deprecated`, together with the
`REGISTER_SELECT` encoder mode that only it used.

*Commit `76f71ae` — "fix new FRAM chip startup - default to back register"*

## 9. Continuous per-sensor velocity

**Background.** No velocity was computed on the active path: every note is sent
with a fixed velocity of 100. `getOnVelocity()` / `getOffVelocity()` and
`checkJackStates()` exist but are never called.

**Change.** `readSensors()` now updates `sensorVelocity[numSensors]` on every
scan, for every sensor (new globals in `haptic_harpsichord.ino`). It is not
used for MIDI yet.

- **Units:** reading units (13-bit scale) per second, as `float`. Negative =
  reading falling (key going down), positive = rising.
- **Time step:** measured each scan with `micros()` (`scanDtMicros`), not a fixed
  per-mode value, because scan time varies with the mode, serial printing and
  MIDI traffic. Measured as an unsigned difference, so it stays correct when
  `micros()` (32-bit) wraps around every ~71.6 minutes.
- **Arithmetic:** one float division per scan (`1e6 / scanDtMicros`), then one
  multiply per sensor. Using `float` avoids the 32-bit overflow of
  `delta × 1,000,000` (up to 8.19 × 10⁹). The nRF52840 has a hardware FPU,
  so the cost is a few µs per scan. Memory: 196 bytes for the array.
- **Guards:** velocity is 0 on the first scan (no previous timestamp) and if
  the time step were ever 0.
- **Meaning:** readings are an 8-sample moving average, so `curr − prev` is the
  average slope over the last 8 scans. This is less noisy, but lags the real
  movement by about 4 scans and partly averages out very fast peaks.
- **Long pauses:** after a blocking pause (for example the 100 ms LED flash on
  double-click), the next value is the average slope over that whole pause. It
  is still correct, just not instantaneous.
- **Plotter:** in (hysteretic) debug mode, serial command `v` toggles the plot
  between the position view (reading + thresholds) and a velocity view
  (`printJackVelocity()` in `print_functions.ino`: `V<key>` plus a `Z` zero
  line, auto-scaled). They are separate views because velocity (tens of
  thousands of units/s) and readings (a window of a few hundred to a few
  thousand units) cannot share one axis.

*Commit `3e4e5f0` — "initial velocity commit". Section 10 below supersedes the
time step, arithmetic and guards details above.*

## 10. Velocity: window-based timing, fixed plot range

**Problem.** The velocity in section 9 was very noisy in debug mode. The
difference of two consecutive moving averages, `curr − prev`, is really
`(x_n − x_(n−8)) / 4`, a change over the whole **8-scan window**. But it was
divided by the time of the **last scan only**. In debug mode, serial printing
makes individual scans irregular, so every slow or fast scan produced a velocity
spike even with the key at rest. The plotter's auto-scaling then zoomed in on
that noise, which made it look even larger.

(Averaging instantaneous velocities instead of differencing the averages
would not help: the two are mathematically identical, because the middle terms
cancel and both reduce to `(x_n − x_(n−8))`.)

**Change.**
- **Window timing:** each scan's `micros()` timestamp is kept in `scanTimes[]`,
  a ring buffer indexed like `sensorHistory`. Velocity is now
  `(newest raw − raw from 8 scans ago) / (time between those two scans)`, so the
  time matches the change it is divided by. Irregular scans no longer cause
  spikes. Cost: 32 bytes.
- **Raw samples:** the difference is taken from the raw samples (the one added
  to and the one dropped from the window), converted to the 13-bit scale
  (× `AVG_SIZE / 2^(AVG_SHIFT−1)` = × 2). This avoids the rounding of the
  `>> (AVG_SHIFT − 1)` shift.
- **Validity:** velocity is 0 until the window holds `AVG_SIZE` real scans
  (`scanCount`). `setup()` already primes 8 scans, so it is valid from the
  start of operation.
- `scanDtMicros` (last scan interval) and the new `windowMicros` (window span)
  stay available as diagnostics. `lastScanMicros` / `haveLastScan` were removed.
- **Fixed plot range:** the velocity view prints constant `m` / `M` lines at
  ± `VELOCITY_PLOT_RANGE` (20,000 units/s, in `haptic_harpsichord.ino`), so
  noise at rest is shown at its real size relative to key presses. Faster
  movements still extend the plot. (±100,000 was tried first and was too large.)

*Commit `a211a6b` — "implement velocity computation printout"*

## 11. Velocity: least-squares slope

**Problem.** The velocity in section 10, `(x_n − x_(n−8)) / (t_n − t_(n−8))`,
depends on only two raw samples, the newest and the one from 8 scans ago. The 7
samples in between are ignored, so their information is wasted and the noise of
the two end samples passes straight through.

**Change.** `readSensors()` now fits a straight line through the same 9 raw
samples (the 8 in the window plus the one dropped this scan), against their
measured scan times, and uses its slope:

```
v = Σ w_i · x_i,   w_i = (t_i − t̄) / Σ (t_j − t̄)²
```

- **Weights per scan:** the weights depend only on the scan times, so they are
  computed once per scan and shared by all 49 sensors. Per sensor, the cost is 9
  multiply-adds in float (hardware FPU), a few tens of µs per scan in total.
- **Real timestamps:** the fit uses the actual `scanTimes`, so it stays exact
  when scan intervals are irregular (debug mode).
- **Float precision:** times are taken relative to the current scan ("µs ago"),
  and samples relative to the newest one (the weights sum to zero), so the
  floats stay small and precise. The weights also convert to 13-bit reading
  units per second.
- **Same centre and lag** as before: the middle of the window, about 4 scans.
  Same validity rule: 0 until the window holds 8 real scans.

**Verified** with a simulation of the same algorithm:
- a linear ramp with random scan intervals (0.8–3 ms) is reproduced exactly,
  within 0.01% (rounding of the integer samples)
- at rest with Gaussian noise, the standard deviation is 26% lower than the
  two-sample difference (790 vs 1072 units/s in the test)

(An earlier estimate of "25–30%" had been computed against a 7-interval
difference. Over only the 8 samples in the buffer, the gain would be about 13%.
Including the dropped 9th sample is what gives the 26%.)

## 12. Shorter velocity window, plot range ±15,000

**Change.**
- **Configurable fit length:** new `VELOCITY_WINDOW` in `haptic_harpsichord.ino`,
  the number of raw samples in the least-squares fit (2 to `AVG_SIZE + 1`, checked
  at compile time). Default changed from 9 to **5**. The fit uses the newest
  samples from the history buffer (slots `windex`, `windex − 1`, …). It only
  includes the dropped sample when the window is 9. It is independent of the
  8-sample moving average used for readings and thresholds, which is unchanged.
- **Plot range:** `VELOCITY_PLOT_RANGE` reduced from 20,000 to 15,000.

**Trade-off** (simulation of the same algorithm, 1 ms scans, Gaussian noise at
rest; a linear ramp with irregular scan times is reproduced within 0.1% for all
lengths):

| `VELOCITY_WINDOW` | Centred (scans back) | Noise std at rest | vs. 9 |
|---|---|---|---|
| 9 (before) | 4.0 | 784 | 1.0× |
| 8 | 3.5 | 935 | 1.2× |
| 7 | 3.0 | 1142 | 1.5× |
| 6 | 2.5 | 1445 | 1.8× |
| **5 (default)** | **2.0** | **1907** | **2.4×** |
| 4 | 1.5 | 2699 | 3.4× |
| 3 | 1.0 | 4283 | 5.5× |

Noise grows roughly with the 1.5 power of the shortening, because a shorter fit
has both fewer samples and a shorter time span.

## 13. Separate velocity buffer: window up to 32 samples

**Problem.** The velocity fit read its samples from the moving-average buffer
(`sensorHistory`), which only holds 8 samples (plus the one dropped each scan).
`VELOCITY_WINDOW` therefore could not exceed 9. Making the moving average
longer would also have smoothed the readings, adding delay to note detection.

**Change.**
- **Own buffer:** the velocity has its own ring buffer, `velocityHistory[49][VELOCITY_WINDOW]`,
  with its own timestamps (`velocityTimes`), index (`vindex`, any length, not
  only powers of 2) and fill counter (`velocityCount`). `readSensors()` stores
  each raw sample in both buffers.
- **Range:** `VELOCITY_WINDOW` can be 2 to `VELOCITY_WINDOW_MAX` (32), checked
  at compile time. The default stays at 5.
- **Readings unchanged:** the moving average, readings, thresholds and note
  detection are completely unaffected by the window length.
- `scanTimes` and `scanCount` (section 10) were replaced by the new buffer.
  `scanDtMicros` stays, and `windowMicros` now means the time spanned by the
  velocity fit.
- **Validity:** velocity is 0 until the buffer has been filled once
  (`VELOCITY_WINDOW` scans).
- **Memory:** 49 × `VELOCITY_WINDOW` × 2 bytes plus 4 bytes per timestamp
  (5 → 510 bytes, 32 → about 3.3 KB of the 256 KB RAM).
- **Per scan:** `VELOCITY_WINDOW` multiply-adds per sensor.

**Verified** with a simulation of the same algorithm (1 ms scans). A linear ramp
with irregular scan times is reproduced within 0.04% for every size. Sizes 5 and
9 give exactly the same results as the previous implementation:

| `VELOCITY_WINDOW` | Centred (scans back) | Noise std at rest |
|---|---|---|
| **5 (default)** | 2.0 | 1907 |
| 9 | 4.0 | 784 |
| 12 | 5.5 | 511 |
| 16 | 7.5 | 329 |
| 24 | 11.5 | 178 |
| 32 | 15.5 | 115 |

*Sections 11–13: commit `045f220` — "implement velocity smoothing + own velocity
buffer"*

## 14. Lowest velocity since Note Off, for pluck-peak analysis

**Purpose.** To find out whether the velocity peak when the jack plucks the
string stands out from noise clearly enough to be used for MIDI.

**Change.**
- New `minVelocitySinceRelease[numSensors]` in `haptic_harpsichord.ino`: for each
  key, the lowest (most negative, i.e. fastest downward) velocity since its last
  Note Off.
- Updated in `processHysteretic()` (`jack_state_functions.ino`) on every scan, for
  the active keys. It is reset to 0 at Note Off. So between notes it tracks the
  noise floor, and during a press it captures the pluck peak. This is a
  comparison per key per scan, so it also runs in play mode at negligible cost.
- The debug velocity view shows it as `L<key>`, next to `V<key>`. It is shown as a
  plotter line rather than as text messages, which would break the Serial
  Plotter.

**Not reset** by a register change (`f` / `b`), which releases sounding notes
without going through `processHysteretic()`. The next normal Note Off resets it.

---

## Background: 12-bit vs 13-bit readings

The change in scale was introduced on `MayConcerts` itself, in commit `76a77f7`
"New bit depth" (May 11, 2026), so it is not a `lab-keyboard` change. It is
recorded here because section 4 depends on it.

**Before.** Each scan read every sensor once with `analogRead()`. The ADC is set
to 12 bits (`analogReadResolution(12)`), so readings were whole numbers from 0
to 4095.

**After.** `readSensors()` applies a moving-average filter. For each sensor it
keeps the last `AVG_SIZE` = 8 raw readings in a ring buffer and maintains
their running sum, which reduces noise. A true average would divide the sum by 8,
i.e. shift right by `AVG_SHIFT` = 3. The code shifts by `AVG_SHIFT - 1` = 2
instead, which divides by 4:

```cpp
currSensorReadings[sensorIdx] = sensorSums[sensorIdx] >> (AVG_SHIFT - 1);
```

This is deliberate. It is called **oversampling**. Averaging 8 noisy readings
gives a value that is more precise than any single reading. For example, the
true average might be 1234.5. Dividing by 8 would round that to 1234 and lose
the half; dividing by 4 gives 2469 and keeps it. The result is "twice the
average", from 0 to 8190 (`4095 × 8 >> 2`). That is one extra bit, so 13 bits
of resolution from a 12-bit ADC.

**Consequence.** For the same physical key position, readings are now about
**twice** what they were before. A key that rested at 2000 now rests at about
4000. Every number that is compared with a reading has to be on the same scale:
thresholds, encoder limits, plot limits, velocity scaling, defaults. Commit
`76a77f7` did not update all of them, and section 4 fixes the rest.
`autoCalibrate()` was written for the 13-bit scale and needed no change.

**Second change in the same commit.** `readSensors()` used to swap the
`currSensorReadings` / `prevSensorReadings` pointers at the *end* of the
function. As a result, after each scan, "curr" actually held the *older*
reading. `76a77f7` moved the swap to the start, so "curr" is now really the
newest reading, and a key press is a *falling* reading. This reversed the
direction that the existing threshold tests respond to. That is likely why
earlier commits ("swap note on/off #8") had swapped Note On and Note Off, to
compensate. Thresholds calibrated before this commit are therefore wrong in both
scale and direction, which is why a recalibration is required.
