// Good examples:
// [1] http://seriss.com/people/erco/fltk/#OpenGlSimple
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <GL/glut.h>

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

typedef enum {
    ALIGN_V_TOP,
    ALIGN_V_CENTER,
    ALIGN_V_BOTTOM,
    ALIGN_H_LEFT,
    ALIGN_H_CENTER,
    ALIGN_H_RIGHT
} align_t;

// Defines
#define FRAME_RATE_Hz 24
#define NUM_COLORS 10
#define MAX_LABEL_SIZE 1024
#define BORDER 20
#define FONT GLUT_BITMAP_8_BY_13

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

    void draw(void);
    void draw_text(double x, double y, string s, align_t halign, align_t valign);
    void draw_decorations(void);
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
    // Refresh scaling
    glLoadIdentity();
    glViewport(BORDER, BORDER, w()-2*BORDER, h()-2*BORDER);
    glOrtho(xmin, xmax, ymin, ymax, -1, 1);

    // Clear screen
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Add decorations
    draw_decorations();

    // Plot all datapoints
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

        for (unsigned int n = 0; n < s->x.size(); n++) {
            glVertex2f(s->x[n], s->y[n]);
        }
        glEnd();

        i += 1;
    }
}

void Pivotter::draw_text(double x, double y, string s, align_t halign, align_t valign)
{
    // TODO: Align vertically and horizontally.
    // [1] https://www.programming-techniques.com/2012/05/font-rendering-in-glut-using-bitmap-fonts-with-sample-example.html

    glColor3ub(0x00, 0x00, 0x00);
    // glColor3ub(0xff, 0x00, 0x00);
    glRasterPos2f(x, y);
    /* int w = glutBitmapLength(FONT, (const unsigned char *)s.c_str()); */
    // glPushMatrix();
    // glLoadIdentity();
    for (unsigned int n = 0; n < s.length(); n++) {
        glutBitmapCharacter(FONT, s[n]);
    }
    // glPopMatrix();
}

void Pivotter::draw_decorations(void)
{
    char s[MAX_LABEL_SIZE];

    // Draw outline
    glLineWidth(3.0);
    glColor3ub(0x00, 0x00, 0x00);
    glBegin(GL_LINE_LOOP);
        glVertex2f(xmin, ymin);
        glVertex2f(xmin, ymax);
        glVertex2f(xmax, ymax);
        glVertex2f(xmax, ymin);
    glEnd();

    // Draw ticks

    // y = ax + b;
    // b = y - ax;
    // a = (y1-y0)/(x1-x0);
    // a = (xmax-xmin)
    // b = -a*xmin

    snprintf(s, MAX_LABEL_SIZE, "x=(%.3g, %.3g) y=(%.3g, %.3g)", xmin, xmax, ymin, ymax);
    draw_text(xmin, ymin, string(s), ALIGN_H_CENTER, ALIGN_V_BOTTOM);
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

void print_help(void)
{
    cout << "Usage:" << endl
        << "    pivotter [OPTIONS] FILENAME.csv" << endl << endl
        << "Options:" << endl
        << "    -y YCOLUMN     Column to plot along y-axis" << endl
        << "    -x XCOLUMN     Column to plot along x-axis" << endl
        << "    -u HUECOLUMN   Column to determine the plot hue (color)" << endl
        << "    -l SKIPROWS    Number of lines to skip before the data [1]" << endl
        << "    -n HEADERROW   Row where the header text is found [0]" << endl
        << "    -s             Plot as scatter plot instead of line plot" << endl
        << "    -h             Print this message" << endl
        << endl;
}

int main(int argc, char *argv[]) 
{
    int opt;

    int xcol = -1;
    int ycol = -1;
    int huecol = -1;
    bool scatter = false;
    int skiprows = 1;
    int headerrow = 0;
    string filename;

    // Optional arguments
    while ((opt = getopt(argc, argv, "x:y:u:l:n:sh")) != EOF) {
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

        case 'l':
            skiprows = stoi(string(optarg));
            break;

        case 'n':
            headerrow = stoi(string(optarg));
            break;

        case 's':
            scatter = true;
            break;

        case 'h':
            // Fall through
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

    // Initialize file source
    Source source(filename, xcol, ycol, huecol, skiprows, headerrow);

    // If no y-parameter is specified, print the header info to help the user
    // specify arguments.
    if (ycol < 0) {
        print_help();
        source.print_header();
        return 2;
    }

    // Initialize GUI objects
    Fl_Window win(500, 300, "Pivotter");
    Pivotter pivotter(&source, 10, 10, win.w()-20, win.h()-20);

    // Set up GUI objects
    if (scatter) {
        pivotter.set_style_scatter();
    }
    else {
        pivotter.set_style_line();
    }
    win.end();
    win.resizable(pivotter);
    win.show();

    // Run the program
    auto ret = Fl::run();
    return ret;
}
