// Good examples:
// [1] http://seriss.com/people/erco/fltk/#OpenGlSimple
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "source.h"

using namespace std;

#define FRAME_RATE_Hz 24
#define NUM_COLORS 10

GLubyte color_lut[NUM_COLORS][3] = {
    { 0x1f, 0x77, 0xb4 },
    { 0xff, 0x7f, 0x0e },
    { 0x2c, 0xa0, 0x2c },
    { 0xd6, 0x27, 0x28 },
    { 0x94, 0x67, 0xbd },
    { 0x8c, 0x56, 0x4b },
    { 0xe3, 0x77, 0xc2 },
    { 0x7f, 0x7f, 0x7f },
    { 0xbc, 0xbd, 0x22 },
    { 0x17, 0xbe, 0xcf },
};

typedef struct {
    vector<double> x;
    vector<double> y;
    GLubyte *color;
} xy_t;

typedef unordered_map<string, xy_t> data_t;

void get_sample(double *x, double *y)
{
    *x = rand();
    *y = rand();
}

class Pivotter : public Fl_Gl_Window
{
private:
    Source *source;
    data_t data;
    int num_samples = 0;
    int colorcnt;
    double xmin;
    double xmax;
    double ymin;
    double ymax;

    GLfloat xscale(double x);
    GLfloat yscale(double y);
    void draw();
    static void timer_cb(void *handle);

public:
    // Constructor
    Pivotter(Source *this_source, int X, int Y, int W, int H, const char*L=0);
    void reset(void);
    void add(double x, double y, string hue);
};

GLfloat Pivotter::xscale(double x)
{
    return -w() + (x-xmin)*(w()+w())/(xmax-xmin);
}

GLfloat Pivotter::yscale(double y)
{
    return -h() + (y-ymin)*(h()+h())/(ymax-ymin);
}

void Pivotter::draw()
{
    if (!valid()) {
        glLoadIdentity();
        glViewport(0,0, w(), h());
        glOrtho(-w(), w(), -h(), h(), -1, 1);
    }

    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // Plot all lines
    int i = 0;
    for (auto it = data.cbegin(); it != data.cend(); it++) {
        string hue = it->first;

        // Draw line
        glColor3ub(data[hue].color[0], data[hue].color[1], data[hue].color[2]);
        // glBegin(GL_LINE_STRIP);
        glPointSize(6.0);
        glBegin(GL_POINTS);
        for (int n = 0; n < data[hue].x.size(); n++) {
            glVertex2f(xscale(data[hue].x[n]), yscale(data[hue].y[n]));
        }
        glEnd();

        i += 1;
    }
}

void Pivotter::timer_cb(void *handle)
{
    Pivotter *h = (Pivotter *)handle;
    double x, y;
    string hue;

    bool ok;
    while (1) {
        h->source->get_sample(&x, &y, &hue, &ok);
        if (ok) {
            h->add(x, y, hue);
            h->redraw();
        }
        else {
            break;
        }
    }
    Fl::repeat_timeout(1.0/FRAME_RATE_Hz, timer_cb, handle);
}

Pivotter::Pivotter(Source *this_source, int X, int Y, int W, int H, const char*L) : Fl_Gl_Window(X, Y, W, H, L)
{
    source = this_source;
    reset();
    Fl::add_timeout(1.0/FRAME_RATE_Hz, timer_cb, (void *)this);
    end();
}

void Pivotter::reset(void)
{
    data.clear();
    num_samples = 0;
    colorcnt = 0;
    xmin = 0.0;
    xmax = 0.0;
    ymin = 0.0;
    ymax = 0.0;
}

void Pivotter::add(double x, double y, string hue)
{
    if (num_samples == 0) {
        xmin = x;
        xmax = x;
        ymin = y;
        ymax = y;
    }
    else {
        if (xmax < x) {
            xmax = x;
        }
        if (ymax < y) {
            ymax = y;
        }
        if (x < xmin) {
            xmin = x;
        }
        if (y < ymin) {
            ymin = y;
        }
    }

    if (data.find(hue) == data.end()) {
        data[hue].color = color_lut[colorcnt % NUM_COLORS];
        colorcnt += 1;
    }
    data[hue].x.push_back(x);
    data[hue].y.push_back(y);
    num_samples += 1;

    draw();
}

int main(int argc, const char *argv[]) 
{
    if (argc != 5) {
        cout << "Usage:" << endl;
        cout << "    pivotter XCOLUMN YCOLUMN HUECOLUMN FILENAME" << endl;
        return 1;
    }
    int xcol = stoi(string(argv[1]));
    int ycol = stoi(string(argv[2]));
    int huecol = stoi(string(argv[3]));
    string filename = string(argv[4]);

    Fl_Window win(500, 300, "Pivotter");
    Source source(filename, xcol, ycol, huecol);
    Pivotter pivotter(&source, 10, 10, win.w()-20, win.h()-20);

    win.end();
    win.resizable(pivotter);
    win.show();

    return(Fl::run());
}
