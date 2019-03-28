import matplotlib

matplotlib.use("TkAgg")

import os
import sys
import pprint
import json

import numpy as np
import matplotlib.pyplot as plt

class Pivot(object):
    def __init__(self, x, y, hue=None, col=None, row=None, height=2.5, aspect=1.5):
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

        # self.fig.tight_layout()

    def update_figure(self):
        M, N = len(self.rows), len(self.cols)

        self.fig.clear()
        self.fig.set_size_inches(self.height*self.aspect*N, self.height*M)

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

                    xy[row][col][hue], = ax.plot(xvalues, yvalues, label=hue, marker="o")

                ax.set_xlabel(self.x)
                ax.set_ylabel(self.y)
                ax.grid(True)
        ax.legend()

        self.xy = xy

def main(input_file):
    p = Pivot(
        x = "frequency",
        y = "power",
        hue = "id",
        col = "modulation",
        height = 3,
    )

    with open(input_file, "r") as f:
        rows = [line.rstrip().split(",") for line in f]

    p.parse_header(rows[0])
    for r in rows[1:]:
        p.add_point(r)
        print(r)
        # input()

    for row in p.rows:
        for col in p.cols:
            for hue in p.hues:
                print(row, col, hue, p.xy[row][col][hue].get_xdata(), p.xy[row][col][hue].get_ydata())

    # p.plot()
    # p.fig.canvas.draw()
    # p.fig.canvas.flush_events()
    # plt.draw()
    p.fig.canvas.draw_idle()
    plt.show()

if __name__ == "__main__":
    main("test.csv")
