#ifndef PIVOT_H
#define PIVOT_H

#include <vector>
#include <unordered_map>
using namespace std;

typedef struct {
    vector<double> x;
    vector<double> y;
} xy_t;

//                    hue
typedef unordered_map<string, xy_t> data_t;

data_t get_data(void);

#endif
