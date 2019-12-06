#include <iostream>
using namespace std;

static struct {
    string filename;
    int skiprows;
    int headerrow;
    int xcol;
    int ycol;
    int huecol;
} settings;

int test_init(void)
{
    settings.filename = "data/iris.csv";
    settings.skiprows = 1;
    settings.headerrow = 1;
    settings.xcol = 0;
    settings.ycol = 1;
    settings.huecol = 2;
}

int main(int argc, const char *argv[]) 
{
    test_init();
    return 0;
}
