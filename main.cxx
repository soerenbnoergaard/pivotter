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
#include <time.h>

#include "source.h"

using namespace std;

typedef struct {
    int t;
    int n;
} clock_sample_t;

// Debug data
static struct {
    clock_sample_t clk_redraw = {0, 0};
    clock_sample_t clk_add = {0, 0};
    clock_sample_t clk_getsample = {0, 0};
} debug;

// Defines
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

    bool style_scatter;

    void draw();
    static void timer_cb(void *handle);

public:
    // Constructor
    Pivotter(Source *this_source, int X, int Y, int W, int H, const char*L=0);
    void reset(void);
    void add(double x, double y, string hue);
    void set_style_scatter(void);
    void set_style_line(void);
};

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
    style_scatter = true;
}

void Pivotter::draw()
{
    // if (!valid()) {
    //     glLoadIdentity();
    //     glViewport(0,0, w(), h());
    //     glOrtho(-w(), w(), -h(), h(), -1, 1);
    // }
    glLoadIdentity();
    glViewport(0,0, w(), h());
    glOrtho(xmin, xmax, ymin, ymax, -1, 1);

    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // Plot all lines
    int i = 0;
    for (auto it = data.cbegin(); it != data.cend(); it++) {
        string hue = it->first;
        const xy_t *s = &it->second;

        // Draw line
        glColor3ub(s->color[0], s->color[1], s->color[2]);
        if (style_scatter) {
            glPointSize(4.0);
            glBegin(GL_POINTS);
        }
        else {
            glLineWidth(3.0);
            glBegin(GL_LINE_STRIP);
        }

        for (int n = 0; n < s->x.size(); n++) {
            glVertex2f(s->x[n], s->y[n]);
        }
        glEnd();

        i += 1;
    }
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

void Pivotter::set_style_scatter(void)
{
    style_scatter = true;
}

void Pivotter::set_style_line(void)
{
    style_scatter = false;
}

void Pivotter::timer_cb(void *handle)
{
    Pivotter *h = (Pivotter *)handle;
    double x = 0.0;
    double y = 0.0;
    string hue = "";
    clock_t start;

    bool ok;
    while (1) {
        start = clock();
        h->source->get_sample(&x, &y, &hue, &ok);
        if (ok) {
            debug.clk_getsample.t += clock() - start;
            debug.clk_getsample.n += 1;
            
            start = clock();
            h->add(x, y, hue);
            debug.clk_add.t += clock() - start;
            debug.clk_add.n += 1;

            start = clock();
            h->redraw();
            debug.clk_redraw.t += clock() - start;
            debug.clk_redraw.n += 1;
        }
        else {
            break;
        }
    }
    Fl::repeat_timeout(1.0/FRAME_RATE_Hz, timer_cb, handle);
}

void print_help(void)
{
    cout << "Usage: " << endl
        << "    pivotter [OPTIONS] FILENAME.csv" << endl << endl
        << "Options:" << endl
        << "    -x XCOLUMN" << endl
        << "    -y YCOLUMN" << endl
        << "    -u HUECOLUMN" << endl
        << endl;
}

int main(int argc, char *argv[]) 
{
    int opt;

    int xcol = -1;
    int ycol = -1;
    int huecol = -1;
    string filename;

    // Optional arguments
    while ((opt = getopt(argc, argv, "x:y:u:")) != EOF) {
        switch (opt) {
        case 'x':
            xcol = stoi(string(optarg));
            break;

        case 'y':
            ycol = stoi(string(optarg));
            break;

        case 'u':
            huecol = stoi(string(optarg));
            break;

        case '?':
            // Fall through
        default:
            print_help();
            return 2;
        }
    }

    // Positional arguments
    if (argv[optind] == NULL) {
        print_help();
        return 2;
    }
    filename = string(argv[optind]);

    // Check the input arguments
    if (ycol < 0) {
        print_help();
        return 2;
    }

    Fl_Window win(500, 300, "Pivotter");
    Source source(filename, xcol, ycol, huecol);
    Pivotter pivotter(&source, 10, 10, win.w()-20, win.h()-20);

    win.end();
    win.resizable(pivotter);
    win.show();

    auto ret = Fl::run();
    /* cout << "Get sample: " << (1.0*debug.clk_getsample.t)/debug.clk_getsample.n << endl; */
    /* cout << "Add:        " << (1.0*debug.clk_add.t)/debug.clk_add.n << endl; */
    /* cout << "Redraw:     " << (1.0*debug.clk_redraw.t)/debug.clk_redraw.n << endl; */
    return ret;
}
