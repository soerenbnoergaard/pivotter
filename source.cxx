#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "source.h"

using namespace std;

Source::Source(string this_filename, int this_xcol, int this_ycol, int this_huecol)
{
    filename = this_filename;
    skiprows = 1;
    headerrow = 1;
    xcol = this_xcol;
    ycol = this_ycol;
    huecol = this_huecol;
    delimiter = ',';

    rowcnt = 0;
    filehandle = ifstream(filename);
}

void Source::get_sample(double *x, double *y, string *hue, bool *ok)
{
    string line;
    string word;

    *ok = false;
    filehandle.clear();

    if (!getline(filehandle, line)) {
        return;
    }

    if (rowcnt == headerrow) {
        rowcnt += 1;
        return;
    }
    else if (rowcnt < skiprows) {
        rowcnt += 1;
        return;
    }

    istringstream iss(line);
    int missingcnt = (xcol >= 0) + (ycol >= 0) + (huecol >= 0);

    // Split CSV line
    try {
        for (int colcnt = 0; getline(iss, word, delimiter); colcnt++) {
            if (xcol == colcnt) {
                *x = stod(word);
                missingcnt -= 1;
            }
            else if (ycol == colcnt) {
                *y = stod(word);
                missingcnt -= 1;
            }
            else if (huecol == colcnt) {
                *hue = word;
                missingcnt -= 1;
            }
        }
    }
    catch (...) {
        rowcnt += 1;
        return;
    }

    if (missingcnt != 0) {
        cout << "Not enough columns in line " << endl;
        rowcnt += 1;
        return;
    }

    *ok = true;

    // If no x-column is selected, use a running sample count as x-axis.
    if (xcol < 0) {
        *x = rowcnt - skiprows;
    }

    rowcnt += 1;
    return;
}
