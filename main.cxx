#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Chart.H>
#include "pivot.h"
using namespace std;

int main(void)
{
    Fl_Window *win = new Fl_Window(1000, 480);
    Fl_Chart *chart0 = new Fl_Chart(20, 20, win->w()-40, win->h()-40, NULL);

    chart0->type(FL_LINE_CHART);
    chart0->autosize(true);
    chart0->color(FL_WHITE);

    data_t xy = get_data();
    xy_t z = xy["setosa"];

    for (int n = 0; n < z.x.size(); n++) {
        chart0->add(z.y[n], NULL, FL_BLUE);
    }

    win->resizable(win);
    win->show();

    return Fl::run();
}

