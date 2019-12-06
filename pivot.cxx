#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "pivot.h"

using namespace std;

static struct {
    string filename;
    int skiprows;
    int headerrow;
    int xcol;
    int ycol;
    int huecol;
    char delimiter;
} settings;

int test_init(void)
{
    settings.filename = "data/iris.csv";
    settings.skiprows = 1;
    settings.headerrow = 1;
    settings.xcol = 0;
    settings.ycol = 2;
    settings.huecol = 4;
    settings.delimiter = ',';
}

int print_data(data_t xy)
{
    // Print out the xy structure
    for (auto it = xy.cbegin(); it != xy.cend(); ++it) {
        string hue = it->first;
        for (int n = 0; n < xy[hue].x.size(); n++) {
            double x = xy[hue].x[n];
            double y = xy[hue].y[n];

            cout << it->first << " " << x << " " << y << endl;
        }
    }
    return 0; 
}

data_t get_data(void) 
{
    test_init();

    ifstream infile(settings.filename);
    string line;
    string word;

    data_t xy;

    for (int rowcnt = 0; getline(infile, line); rowcnt++) {
        if (rowcnt == settings.headerrow) {
            // TODO: Handle header row for legend
            continue;
        }
        else if (rowcnt < settings.skiprows) {
            continue;
        }

        // Split CSV line
        istringstream iss(line);
        string hue;
        double x;
        double y;
        int missingcnt = 3;

        for (int colcnt = 0; getline(iss, word, settings.delimiter); colcnt++) {
            if (settings.xcol == colcnt) {
                x = stod(word);
                missingcnt -= 1;
            }
            else if (settings.ycol == colcnt) {
                y = stod(word);
                missingcnt -= 1;
            }
            else if (settings.huecol == colcnt) {
                hue = word;
                missingcnt -= 1;
            }
        }

        if (missingcnt != 0) {
            cout << "Not enough columns in line" << endl;
            continue;
        }

        // Add point to data structure
        xy[hue].x.push_back(x);
        xy[hue].y.push_back(y);
    }

    /* print_data(xy); */
    /* plot(xy["setosa"].x, xy["setosa"].y); */

    return xy;
}
