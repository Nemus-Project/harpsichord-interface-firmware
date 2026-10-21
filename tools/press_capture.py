#!/usr/bin/env python3
"""
Capture key presses from the haptic harpsichord firmware and plot them.

The firmware must be in calibration (debug) mode with the key to capture
selected on the encoder. Close the Arduino IDE Serial Monitor / Plotter first:
only one program can use the serial port at a time.

Monitor (recommended): a live plot that replaces the Arduino Serial Plotter.
Press r in the window to record a press; each recording is saved to captures/
automatically and shown. l cycles the label, v switches position/velocity
view, p turns the stream on/off, q quits:

    python3 tools/press_capture.py monitor --port /dev/cu.usbmodem1101

Capture a fixed number of presses from the terminal (no live plot):

    python3 tools/press_capture.py capture --port /dev/cu.usbmodem1101 --label soft
    python3 tools/press_capture.py capture --port /dev/cu.usbmodem1101 --label hard --count 10

Plot saved captures (one figure each, or overlaid):

    python3 tools/press_capture.py plot captures/*.csv
    python3 tools/press_capture.py plot captures/*soft*.csv --overlay --window 9

Live capture needs pyserial (pip install pyserial); plotting needs numpy and
matplotlib.

Velocity is recomputed offline from the raw samples with the same method as
the firmware: least-squares slope over a window of samples, against the
measured scan times, in 13-bit reading units per second. Offline it is plotted
at the centre of its window (no lag), unlike the firmware, which can only use
past samples.
"""

import argparse
import datetime
import os
import re
import sys

import numpy as np

BAUD = 115200


# --------------------------------------------------------------------------
# Reading and writing captures

def parse_capture(lines):
    """Parse the lines between [CAPTURE] BEGIN and END into (meta, data)."""
    meta = {}
    rows = []
    for line in lines:
        line = line.strip()
        if not line or line.startswith("t_us"):
            continue
        if line.startswith("#"):
            key, _, value = line[1:].partition(":")
            meta[key.strip()] = value.strip()
        else:
            rows.append([int(v) for v in line.split(",")])
    data = np.array(rows, dtype=np.int64)
    return meta, data


def load_capture(path):
    with open(path) as f:
        return parse_capture(f.readlines())


def save_capture(meta, data, out_dir):
    os.makedirs(out_dir, exist_ok=True)
    stamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
    label = re.sub(r"[^A-Za-z0-9_-]+", "-", meta.get("label", "")).strip("-")
    name = f"{stamp}_key{meta.get('key', 'x')}" + (f"_{label}" if label else "") + ".csv"
    path = os.path.join(out_dir, name)
    with open(path, "w") as f:
        for k, v in meta.items():
            f.write(f"# {k}: {v}\n")
        f.write("t_us,raw,reading,note_on\n")
        for row in data:
            f.write(",".join(str(v) for v in row) + "\n")
    return path


# --------------------------------------------------------------------------
# Analysis

def velocity(t_us, raw, window, scale):
    """Least-squares slope over `window` samples, centred, in reading units/s.

    Returns (t_centre_us, v). Same fit as the firmware (raw samples against
    measured times, times scale x raw), but each value is placed at the centre
    of its window.
    """
    t = t_us.astype(float)
    x = raw.astype(float)
    n = len(x) - window + 1
    if n <= 0:
        return np.array([]), np.array([])
    v = np.empty(n)
    tc = np.empty(n)
    for i in range(n):
        tw = t[i:i + window]
        xw = x[i:i + window]
        dt = tw - tw.mean()
        v[i] = scale * 1e6 * np.dot(dt, xw - xw.mean()) / np.dot(dt, dt)
        tc[i] = tw.mean()
    return tc, v


def summary(meta, data):
    t = data[:, 0]
    dt = np.diff(t)
    pre = int(meta.get("pretrigger", 0))
    print(f"  key {meta.get('key')}  label '{meta.get('label', '')}'  "
          f"samples {len(t)}  duration {t[-1] / 1000:.0f} ms  end {meta.get('end')}")
    print(f"  scan time: median {np.median(dt):.0f} us, min {dt.min()} us, max {dt.max()} us")
    print(f"  pretrigger {pre} samples; note on: {'yes' if data[:, 3].any() else 'NO'}")


# --------------------------------------------------------------------------
# Plotting

def note_on_spans(t_ms, note_on):
    """(start, end) times in ms of the periods where note_on is 1."""
    spans = []
    start = None
    for ti, on in zip(t_ms, note_on):
        if on and start is None:
            start = ti
        elif not on and start is not None:
            spans.append((start, ti))
            start = None
    if start is not None:
        spans.append((start, t_ms[-1]))
    return spans


def draw_captures(fig, captures, window_override, overlay):
    """Draw captures into `fig` (cleared first): one capture, or several overlaid."""
    fig.clf()
    axes = fig.subplots(3, 1)
    if overlay:
        fig.suptitle("Overlay, aligned at the trigger (release threshold crossing)")
    for name, meta, data in captures:
        window = window_override or int(meta.get("velocity_window", 5))
        scale = float(meta.get("reading_scale", 2))
        t_us, raw, reading, note_on = data.T
        pre = int(meta.get("pretrigger", 0))
        t0 = t_us[pre] if overlay else 0          # overlay: time 0 = trigger
        t_ms = (t_us - t0) / 1000.0
        tv_us, v = velocity(t_us, raw, window, scale)
        tv_ms = (tv_us - t0) / 1000.0
        reading_at_v = np.interp(tv_us, t_us, reading)

        if not overlay:
            title = f"{os.path.basename(name)}  (key {meta.get('key')}, velocity window {window})"
            if meta.get("label"):
                title += f"  [{meta['label']}]"
            fig.suptitle(title)
        label = os.path.basename(name) if overlay else None
        pluck = float(meta.get("pluck_threshold", "nan"))
        release = float(meta.get("release_threshold", "nan"))

        ax = axes[0]
        line, = ax.plot(t_ms, reading, label=label or "reading")
        if not overlay:
            ax.plot(t_ms, raw * scale, color="0.75", lw=0.6, zorder=0, label="raw x scale")
            for a, b in note_on_spans(t_ms, note_on):
                for axx in axes[:2]:
                    axx.axvspan(a, b, color="tab:green", alpha=0.12)
        color = line.get_color()
        ax.axhline(pluck, color=color if overlay else "tab:green", ls="--", lw=0.8)
        ax.axhline(release, color=color if overlay else "tab:purple", ls=":", lw=0.8)

        axes[1].plot(tv_ms, v, color=color, label=label)
        axes[2].plot(reading_at_v, v, color=color, lw=0.8, label=label)
        axes[2].axvline(pluck, color=color if overlay else "tab:green", ls="--", lw=0.8)
        axes[2].axvline(release, color=color if overlay else "tab:purple", ls=":", lw=0.8)

    _label_axes(axes, overlay)
    axes[0].legend(loc="lower left", fontsize=7 if overlay else 8)
    fig.tight_layout()


def plot_captures(captures, window_override, overlay):
    import matplotlib.pyplot as plt

    if overlay:
        draw_captures(plt.figure(figsize=(11, 10)), captures, window_override, True)
    else:
        for c in captures:
            draw_captures(plt.figure(figsize=(11, 10)), [c], window_override, False)
    plt.show()


def _label_axes(axes, overlay):
    xt = "time from trigger (ms)" if overlay else "time (ms)"
    axes[0].set_ylabel("reading (13-bit)")
    axes[0].set_xlabel(xt)
    axes[0].set_title("Position  (dashed = pluck threshold, dotted = release, green = note on)", fontsize=9)
    axes[1].set_ylabel("velocity (units/s)")
    axes[1].set_xlabel(xt)
    axes[1].axhline(0, color="0.5", lw=0.6)
    axes[1].set_title("Velocity (least squares, centred; negative = key going down)", fontsize=9)
    axes[2].set_xlabel("reading (13-bit)  <- pressed      rest ->")
    axes[2].set_ylabel("velocity (units/s)")
    axes[2].axhline(0, color="0.5", lw=0.6)
    axes[2].set_title("Velocity vs position: a pluck at a fixed position lines up vertically", fontsize=9)
    for ax in axes:
        ax.grid(alpha=0.3)


# --------------------------------------------------------------------------
# Live capture

def read_one_capture(ser):
    """Read serial lines until a complete [CAPTURE] BEGIN ... END block."""
    lines = None
    while True:
        raw = ser.readline()
        if not raw:
            continue  # timeout: keep waiting for the press
        line = raw.decode("ascii", errors="replace").strip()
        if line == "[CAPTURE] BEGIN":
            lines = []
        elif line == "[CAPTURE] END" and lines is not None:
            return lines
        elif lines is not None:
            lines.append(line)
        elif line.startswith("[CAPTURE]"):
            print(line)


def capture(args):
    try:
        import serial
    except ImportError:
        sys.exit("pyserial is needed for live capture: pip install pyserial")

    saved = []
    with serial.Serial(args.port, BAUD, timeout=1) as ser:
        ser.reset_input_buffer()
        try:
            for n in range(args.count):
                print(f"[{n + 1}/{args.count}] arming capture...")
                ser.write(b"r")
                lines = read_one_capture(ser)
                meta, data = parse_capture(lines)
                meta["label"] = args.label
                meta["captured"] = datetime.datetime.now().isoformat(timespec="seconds")
                path = save_capture(meta, data, args.out_dir)
                print(f"  saved {path}")
                summary(meta, data)
                saved.append(path)
        except KeyboardInterrupt:
            ser.write(b"r")  # cancel the armed capture on the board
            print("\ninterrupted, capture cancelled")

    if saved and not args.no_plot:
        plot_captures([(p, *load_capture(p)) for p in saved], args.window, overlay=len(saved) > 1)


# --------------------------------------------------------------------------
# Monitor: live plot (replaces the Arduino Serial Plotter) + automatic saving

PLOT_TOKEN = re.compile(r"^([A-Za-z]+\d*):(-?\d+(?:\.\d+)?)$")
MONITOR_KEYS = ("r", "v", "p", "l", "q")
DEFAULT_LABELS = "soft,normal,hard,very-hard,no-pluck,barely-pluck"


def parse_plot_line(line):
    """Parse a plotter line 'name:value name:value ...' into a dict, or None."""
    tokens = line.split()
    if not tokens:
        return None
    values = {}
    for tok in tokens:
        m = PLOT_TOKEN.match(tok)
        if not m:
            return None
        values[m.group(1)] = float(m.group(2))
    return values


def serial_reader(ser, q, stop):
    """Background thread: put every serial line into the queue."""
    while not stop.is_set():
        try:
            raw = ser.readline()
        except Exception as e:  # board unplugged, port closed
            q.put(("error", str(e)))
            return
        if raw:
            q.put(("line", raw.decode("ascii", errors="replace").strip()))


def monitor(args):
    try:
        import serial
    except ImportError:
        sys.exit("pyserial is needed for the monitor: pip install pyserial")
    import collections
    import queue
    import threading
    import time
    import matplotlib.pyplot as plt

    # Free the monitor's keys from matplotlib's default shortcuts
    for name in list(plt.rcParams):
        if name.startswith("keymap."):
            plt.rcParams[name] = [k for k in plt.rcParams[name] if k not in MONITOR_KEYS]

    labels = [l.strip() for l in args.labels.split(",") if l.strip()] or [""]
    st = {"label": 0, "armed": False, "receiving": False, "saved": 0, "error": None}

    ser = serial.Serial(args.port, BAUD, timeout=0.2)
    ser.reset_input_buffer()
    q = queue.Queue()
    stop = threading.Event()
    threading.Thread(target=serial_reader, args=(ser, q, stop), daemon=True).start()

    def send(c):
        ser.write(c.encode())

    def on_key(event):
        if event.key == "r":
            send("r")  # the firmware arms, or cancels if already armed
        elif event.key == "v":
            send("v")
        elif event.key == "p":
            send("p")
        elif event.key == "l":
            st["label"] = (st["label"] + 1) % len(labels)
        elif event.key == "q":
            plt.close("all")

    plt.ion()
    live = plt.figure("press_capture monitor - live", figsize=(11, 5))
    live.canvas.mpl_connect("key_press_event", on_key)
    ax = live.add_subplot(111)
    capfig = None

    buffers = {}        # series name -> deque of values
    lines = {}          # series name -> Line2D
    limits = {}         # "m" / "M" fixed scale lines from the firmware
    xs = collections.deque(maxlen=args.points)
    sample_no = 0
    last_plot = None
    started = time.time()
    stream_checked = False
    capture_lines = None

    try:
        while plt.fignum_exists(live.number):
            # Handle everything received since the last frame
            while True:
                try:
                    kind, line = q.get_nowait()
                except queue.Empty:
                    break
                if kind == "error":
                    st["error"] = line
                    print(f"serial error: {line}")
                    continue
                if capture_lines is not None:
                    if line == "[CAPTURE] END":
                        meta, data = parse_capture(capture_lines)
                        meta["label"] = labels[st["label"]]
                        meta["captured"] = datetime.datetime.now().isoformat(timespec="seconds")
                        path = save_capture(meta, data, args.out_dir)
                        st["saved"] += 1
                        st["armed"] = st["receiving"] = False
                        capture_lines = None
                        print(f"saved {path}")
                        summary(meta, data)
                        if capfig is None or not plt.fignum_exists(capfig.number):
                            capfig = plt.figure("press_capture monitor - last capture", figsize=(11, 10))
                            capfig.canvas.mpl_connect("key_press_event", on_key)
                        draw_captures(capfig, [(path, meta, data)], args.window, False)
                        capfig.canvas.draw_idle()
                    else:
                        capture_lines.append(line)
                    continue
                if line == "[CAPTURE] BEGIN":
                    capture_lines = []
                    st["receiving"] = True
                    continue

                values = parse_plot_line(line)
                if values is not None:
                    last_plot = time.time()
                    limits = {k: values.pop(k) for k in ("m", "M") if k in values}
                    if set(values) != set(buffers):   # key or view changed: restart
                        ax.cla()
                        buffers = {k: collections.deque(maxlen=args.points) for k in values}
                        lines = {k: ax.plot([], [], label=k, lw=1)[0] for k in values}
                        xs.clear()
                        ax.legend(loc="upper left", fontsize=8)
                        ax.grid(alpha=0.3)
                        ax.set_xlabel("sample")
                    xs.append(sample_no)
                    sample_no += 1
                    for k, v in values.items():
                        buffers[k].append(v)
                    continue

                if line:
                    print(line)
                    if line.startswith("[CAPTURE] armed"):
                        st["armed"] = True
                    elif line.startswith("[CAPTURE] cancelled"):
                        st["armed"] = False

            # Make sure the board is streaming plot data (toggle once if not)
            if not stream_checked and time.time() - started > 1.5:
                stream_checked = True
                if last_plot is None and not st["armed"]:
                    send("p")
                    print("no plot data: sent p to turn the plot stream on")

            # Redraw the live plot
            for k, line2d in lines.items():
                line2d.set_data(xs, buffers[k])
            if xs:
                ax.set_xlim(xs[0], max(xs[-1], xs[0] + 1))
                if "m" in limits and "M" in limits:
                    lo = min([limits["m"]] + [min(b) for b in buffers.values()])
                    hi = max([limits["M"]] + [max(b) for b in buffers.values()])
                else:
                    lo = min(min(b) for b in buffers.values())
                    hi = max(max(b) for b in buffers.values())
                pad = (hi - lo) * 0.05 or 1
                ax.set_ylim(lo - pad, hi + pad)

            if st["error"]:
                status = f"SERIAL ERROR: {st['error']}"
            elif st["receiving"]:
                status = "receiving capture..."
            elif st["armed"]:
                status = "ARMED - press and release the key"
            elif last_plot is None or time.time() - last_plot > 2:
                status = "no data - is the board in calibration mode?"
            else:
                status = "live"
            live.suptitle(
                f"label: {labels[st['label']] or '-'}    saved: {st['saved']}    {status}\n"
                "r = record press    l = next label    v = position/velocity    p = stream on/off    q = quit",
                fontsize=10)
            plt.pause(0.05)
    finally:
        if st["armed"]:
            send("r")  # cancel the armed capture on the board
        stop.set()
        ser.close()


def plot(args):
    captures = []
    for path in args.files:
        meta, data = load_capture(path)
        print(path)
        summary(meta, data)
        captures.append((path, meta, data))
    plot_captures(captures, args.window, args.overlay)


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="command", required=True)

    c = sub.add_parser("capture", help="capture presses from the board")
    c.add_argument("--port", required=True, help="serial port, e.g. /dev/cu.usbmodem1101")
    c.add_argument("--label", default="", help="press type, e.g. soft, hard, no-pluck")
    c.add_argument("--count", type=int, default=1, help="number of presses to capture")
    c.add_argument("--out-dir", default="captures", help="folder for the CSV files")
    c.add_argument("--window", type=int, default=None, help="velocity window for the plot (default: from the file)")
    c.add_argument("--no-plot", action="store_true", help="save only, do not plot")
    c.set_defaults(func=capture)

    m = sub.add_parser("monitor", help="live plot + record presses with r, saved automatically")
    m.add_argument("--port", required=True, help="serial port, e.g. /dev/cu.usbmodem1101")
    m.add_argument("--labels", default=DEFAULT_LABELS,
                   help=f"comma-separated press types, cycled with l (default: {DEFAULT_LABELS})")
    m.add_argument("--out-dir", default="captures", help="folder for the CSV files")
    m.add_argument("--window", type=int, default=None, help="velocity window for the capture plot (default: from the file)")
    m.add_argument("--points", type=int, default=1500, help="samples shown in the live plot")
    m.set_defaults(func=monitor)

    q = sub.add_parser("plot", help="plot saved captures")
    q.add_argument("files", nargs="+", help="CSV files saved by 'capture'")
    q.add_argument("--window", type=int, default=None, help="velocity window (default: from the file)")
    q.add_argument("--overlay", action="store_true", help="overlay all files, aligned at the trigger")
    q.set_defaults(func=plot)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
