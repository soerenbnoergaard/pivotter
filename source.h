#ifndef PIVOT_H
#define PIVOT_H

#include <fstream>

using namespace std;

class Source
{
private:
    ifstream filehandle;
    string filename;
    string header;
    int skiprows;
    int headerrow;
    int xcol;
    int ycol;
    int huecol;
    char delimiter;
    int rowcnt;

public:
    Source(string this_filename, int this_xcol, int this_ycol, int this_huecol, int this_skiprows, int this_headerrow);
    void get_sample(double *x, double *y, string *hue, bool *ok);
    void print_header(void);
};


#endif
