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

        self._check_row_key(row)
        self._check_col_key(col)
        self._check_hue_key(hue)

        self.xy[row][col][hue]["x"].append(x)
        self.xy[row][col][hue]["y"].append(y)

    def plot(self):
        M, N = len(self.rows), len(self.cols)

        fig = plt.figure(figsize=[self.height*self.aspect*N, self.height*M])

        i = 0
        for m, row in enumerate(self.rows):
            for n, col in enumerate(self.cols):
                i += 1
                ax = fig.add_subplot(M, N, i)
                for k, hue in enumerate(self.hues):
                    x = self.xy[row][col][hue]["x"]
                    y = self.xy[row][col][hue]["y"]
                    ax.plot(x, y, label=hue)

                if row is None and col is None:
                    title = None
                elif col is None:
                    title = "{:s} = {}".format(self.row, row)
                elif row is not None:
                    title = "{:s} = {}".format(self.col, col)
                else:
                    title = "{:s} = {} | {:s} = {}".format(self.row, row, self.col, col)

                ax.set_title(title, fontsize=8)
                ax.set_xlabel(self.x)
                ax.set_ylabel(self.y)
            ax.legend(title=self.hue, fontsize=8, title_fontsize=8)
        fig.tight_layout()

    def _check_row_key(self, row):
        if row in self.rows:
            # Row is already in use
            return
        
        self.rows.append(row)
        
        # Append an new container for the new row
        self.xy[row] = dict.fromkeys(self.cols)
        for col in self.xy[row].keys():
            self.xy[row][col] = dict.fromkeys(self.hues)

            for hue in self.xy[row][col].keys():
                self.xy[row][col][hue] = {"x": [], "y": []}

    def _check_col_key(self, col):
        if col in self.cols:
            # Row is already in use
            return

        self.cols.append(col)
        
        # Append an new container for the new col
        for row in self.rows:
            self.xy[row][col] = dict.fromkeys(self.hues)
            for hue in self.xy[row][col].keys():
                self.xy[row][col][hue] = {"x": [], "y": []}

    def _check_hue_key(self, hue):
        if hue in self.hues:
            # Row is already in use
            return

        self.hues.append(hue)
        
        # Append an new container for the new hue
        for row in self.rows:
            for col in self.cols:
                self.xy[row][col][hue] = {"x": [], "y": []}

def main(input_file):
    p = Pivot(
        x = "frequency",
        y = "power",
        row = "id",
        hue = "modulation",
    )

    with open(input_file, "r") as f:
        rows = [line.rstrip().split(",") for line in f]

    p.parse_header(rows[0])
    for r in rows[1:]:
        p.add_point(r)
        # print(json.dumps(p.xy, indent=4))
        # input()
    p.plot()
    plt.show()

if __name__ == "__main__":
    main("test.csv")
