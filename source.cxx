#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include "source.h"

using namespace std;

Source::Source(string this_filename, int this_xcol, int this_ycol, int this_huecol, int this_skiprows, int this_headerrow)
{
    filename = this_filename;
    header = "";
    skiprows = this_skiprows;
    headerrow = this_headerrow;
    xcol = this_xcol;
    ycol = this_ycol;
    huecol = this_huecol;
    delimiter = ',';

    rowcnt = 0;
    filehandle = ifstream(filename);

    // Read skip-lines and store the header line
    for (int n = 0; n < skiprows; n++) {
        string line;

        if (!getline(filehandle, line)) {
            throw "Could not read all skiplines!";
        }

        if (rowcnt == headerrow) {
            header = line;
        }
        rowcnt += 1;
    }
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

void Source::print_header(void)
{
    string word;
    istringstream iss(header);

    cout << "Columns:" << endl;

    for (int n = 0; getline(iss, word, delimiter); n++) {
        cout << "    " << setw(2) << n;
        cout << ": " << word << endl;
    }

    cout << endl;
}
