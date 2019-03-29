import matplotlib

matplotlib.use("TkAgg")

import os
import sys
import csv
import json
import time
import glob
import pprint
import random
import threading

import numpy as np
import tkinter as tk
import matplotlib.pyplot as plt

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from collections import OrderedDict

from tkinter import filedialog

DEBUG = False

if os.name == "nt":
    import ctypes
    from ctypes import wintypes
    import os
    import msvcrt

    GENERIC_READ = 0x80000000
    GENERIC_WRITE = 0x40000000

    OPEN_EXISTING = 3
    OPEN_ALWAYS = 4

    ACCESS_MODES = {
        "r": GENERIC_READ,
        "w": GENERIC_WRITE,
        "r+": (GENERIC_READ|GENERIC_WRITE)
    }

    OPEN_MODES = {
        "r": OPEN_EXISTING,
        "w": OPEN_ALWAYS,
        "r+": OPEN_ALWAYS,
    }

    def open_file_nonblocking(filename, access):
        # Removes the b for binary access.
        internal_access = access.replace("b", "")
        access_mode = ACCESS_MODES[internal_access]
        open_mode = OPEN_MODES[internal_access]
        handle = wintypes.HANDLE(ctypes.windll.kernel32.CreateFileW(
            wintypes.LPWSTR(filename),
            wintypes.DWORD(access_mode),
            wintypes.DWORD(2|1),  # File share read and write
            ctypes.c_void_p(0),
            wintypes.DWORD(open_mode),
            wintypes.DWORD(0),
            wintypes.HANDLE(0)
        ))

        try:
            fd = msvcrt.open_osfhandle(handle.value, 0)
        except OverflowError as exc:
            # Python 3.X
            raise OSError("Failed to open file.") from None
            # Python 2
            # raise OSError("Failed to open file.")

        return os.fdopen(fd, access)
else:
    def open_file_nonblocking(filename, access):
        return open(filename, access)

class Pivot(object):
    def __init__(self, x, y, hue=None, col=None, row=None, height=2.5, aspect=1.5, fig=None):
        self.x = x
        self.y = y
        self.hue = hue
        self.col = col
        self.row = row

        self.height = height
        self.aspect = aspect

        self.x_column = None
        self.y_column = None
        self.hue_column = None
        self.col_column = None
        self.row_column = None

        self.hues = []
        self.cols = []
        self.rows = []

        self.xy = {
        }

        if fig is not None:
            self.fig = fig
        else:
            self.fig = plt.figure()

    def __repr__(self):
        s = ""
        s += "x   = {}\n".format(self.x)
        s += "y   = {}\n".format(self.y)
        s += "hue = {}\n".format(self.hue)
        s += "col = {}\n".format(self.col)
        s += "row = {}\n".format(self.row)

        s += "\n"

        s += "x_column   = {}\n".format(self.x_column)
        s += "y_column   = {}\n".format(self.y_column)
        s += "hue_column = {}\n".format(self.hue_column)
        s += "col_column = {}\n".format(self.col_column)
        s += "row_column = {}\n".format(self.row_column)
        return s.rstrip()

    def parse_header(self, lst):
        for n, element in enumerate(lst):
            if self.hue == element:
                self.hue_column = n

            elif self.col == element:
                self.col_column = n

            elif self.row == element:
                self.row_column = n

            elif self.x == element:
                self.x_column = n

            elif self.y == element:
                self.y_column = n

        for name, c in zip(
                [self.x, self.y, self.hue, self.col, self.row],
                [self.x_column, self.y_column, self.hue_column, self.col_column, self.row_column]
                ):
            if (name is not None) and (c is None):
                raise RuntimeError("{:s} not found in header".format(name))

    def add_point(self, lst):
        x = float(lst[self.x_column])
        y = float(lst[self.y_column])

        row = None if self.row is None else lst[self.row_column]
        col = None if self.col is None else lst[self.col_column]
        hue = None if self.hue is None else lst[self.hue_column]

        update_required = False

        if row not in self.rows:
            self.rows.append(row)
            update_required = True

        if col not in self.cols:
            self.cols.append(col)
            update_required = True

        if hue not in self.hues:
            self.hues.append(hue)
            update_required = True

        # update_required = True
        if update_required:
            self.update_figure()

        h = self.xy[row][col][hue]

        h.axes.relim()
        h.axes.autoscale_view(True, True, True)

        h.set_xdata([X for X in h.get_xdata()] + [x])
        h.set_ydata([Y for Y in h.get_ydata()] + [y])

        self.fig.canvas.draw_idle()

    def update_figure(self):
        M, N = len(self.rows), len(self.cols)

        self.fig.clear()

        xy = dict.fromkeys(self.rows)

        i = 0
        ax = None
        for m, row in enumerate(self.rows):
            xy[row] = dict.fromkeys(self.cols)

            for n, col in enumerate(self.cols):
                xy[row][col] = dict.fromkeys(self.hues)

                i += 1
                ax = self.fig.add_subplot(M, N, i, sharex=ax, sharey=ax)

                for k, hue in enumerate(self.hues):
                    try:
                        xvalues = self.xy[row][col][hue].get_xdata()
                        yvalues = self.xy[row][col][hue].get_ydata()
                    except KeyError:
                        xvalues = []
                        yvalues = []

                    xy[row][col][hue], = ax.plot(xvalues, yvalues, label=hue)

                ax.set_xlabel(self.x)
                ax.set_ylabel(self.y)
                ax.grid(True)
        ax.legend(title=self.hue, fontsize=8)

        self.xy = xy

class Gui(tk.Frame):
    def __init__(self, parent, filename):
        self.parent = parent
        super().__init__(parent)

        self.filename = filename

        # Open the input file
        with open_file_nonblocking(self.filename, "r") as f:
            self.header = [X.strip() for X in f.readline().split(",")]

        self.header.append("")

        fr_open = tk.Frame(self)
        fr_inputs = tk.Frame(self)
        fr_buttons = tk.Frame(self)
        fr_plot = tk.Frame(self)

        self.var = OrderedDict([
            ["x", tk.StringVar()],
            ["y", tk.StringVar()],
            ["hue", tk.StringVar()],
            ["col", tk.StringVar()],
            ["row", tk.StringVar()],
        ])

        h = self.header
        N = len(h)
        if N > 0:
            self.var["x"].set(self.header[0])
        if N > 1:
            self.var["y"].set(self.header[1])
        if N > 2:
            self.var["hue"].set(self.header[2])
        if N > 3:
            self.var["col"].set(self.header[3])
        if N > 4:
            self.var["row"].set(self.header[4])

        # OPEN
        tk.Label(fr_open, text="File:").pack(side=tk.LEFT)
        tk.Label(fr_open, text=self.filename).pack(side=tk.LEFT)

        # INPUTS
        for i, (key, value) in enumerate(self.var.items()):
            tk.Label(fr_inputs, text=key).grid(row=i, column=0, sticky=tk.W)
            tk.OptionMenu(fr_inputs, value, *self.header).grid(row=i, column=1, sticky=tk.W)

        # BUTTONS
        tk.Button(fr_buttons, text="Start", command=self.on_start).pack(side=tk.LEFT)
        tk.Button(fr_buttons, text="Stop", command=self.on_stop).pack(side=tk.LEFT)
        tk.Button(fr_buttons, text="Tight layout", command=self.on_fix_layout).pack(side=tk.LEFT)
        tk.Button(fr_buttons, text="Save", command=self.on_save).pack(side=tk.LEFT)

        # PLOT CANVAS
        self.fig = plt.Figure(dpi=100)
        canvas = FigureCanvasTkAgg(self.fig, master=fr_plot)
        canvas.draw()
        canvas.get_tk_widget().pack(fill=tk.BOTH, expand=1)

        # Pack frames
        fr_open.pack(fill=tk.X)
        fr_inputs.pack(fill=tk.X)
        fr_buttons.pack(fill=tk.X)
        fr_plot.pack(fill=tk.BOTH, expand=True)

        # Common handles
        self.pivot = None
        self.thread = threading.Thread(target=self.loop)
        self.thread_stop_event = threading.Event()

    def on_start(self):
        if self.thread.is_alive():
            print("Thread already running")
            return

        x = self.var["x"].get()
        y = self.var["y"].get()
        hue = self.var["hue"].get()
        col = self.var["col"].get()
        row = self.var["row"].get()

        self.pivot = Pivot(
            x = None if x is "" else x,
            y = None if y is "" else y,
            col = None if col is "" else col,
            row = None if row is "" else row,
            hue = None if hue is "" else hue,
            fig = self.fig,
        )

        self.pivot.parse_header(self.header)

        self.thread = threading.Thread(target=self.loop)
        self.thread_stop_event.clear()
        self.thread.start()

    def on_stop(self):
        self.thread_stop_event.set()
        while self.thread.is_alive():
            time.sleep(0.1)

    def on_fix_layout(self):
        self.pivot.fig.tight_layout()
        self.pivot.fig.canvas.draw_idle()

    def on_save(self):
        f = filedialog.asksaveasfilename(title="Save figure", filetypes=[("PNG", ".png"), ("SVG", ".svg"), ("PDF", ".pdf")], defaultextension=".png")

        if not f:
            return

        self.fig.savefig(f)

    def loop(self):
        p = self.pivot

        n = 0
        with open_file_nonblocking(self.filename, "r") as f:
            while True:
                if self.thread_stop_event.is_set():
                    return

                line = f.readline()
                if line:
                    r = [X.strip() for X in line.split(",")]
                    if n > 0:
                        p.add_point(r)
                else:
                    time.sleep(0.2)
                n += 1

def main(args):
    try:
        exec(open("pivotter/version.py").read())
        version = version
    except Exception:
        version = ""

    while 1:
        root = tk.Tk()
        root.title("Pivotter {:s}".format(version))

        if (len(args) >= 2) and (os.path.isfile(args[1])):
            input_file = args[1]
        else:
            input_file = filedialog.askopenfilename(title = "Select file", filetypes = (("CSV files", "*.csv"), ("All files", "*.*")))
            if not input_file:
                return

        Gui(root, input_file).pack(expand=True, fill=tk.BOTH)
        root.mainloop()
        
        args = []

if __name__ == "__main__":
    debug_file = "debug.csv"
    def debug_writer():
        with open(debug_file, "w") as f:
            f.write("x,y,a,b\n")

        i = 0
        while True:
            with open(debug_file, "a") as f:
                f.write(",".join([str(X) for X in [
                    i,
                    random.randint(10, 15),
                    random.randint(0, 2),
                    random.randint(0, 2),
                ]]) + "\n")

            i += 1
            time.sleep(1)

    if DEBUG:
        t = threading.Thread(target=debug_writer)
        t.daemon = True
        t.start()
        del t
        time.sleep(0.5)

        main(["", debug_file])
    else:
        main(sys.argv)
