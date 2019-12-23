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
    VANCHOR_TOP,
    VANCHOR_CENTER,
    VANCHOR_BOTTOM,
} vanchor_t;

typedef enum {
    HANCHOR_LEFT,
    HANCHOR_CENTER,
    HANCHOR_RIGHT
} hanchor_t;

// Defines
#define FRAME_RATE_Hz 24
#define NUM_COLORS 10
#define MAX_LABEL_SIZE 1024

// #define BORDER 50
#define BORDER_LEFT 50
#define BORDER_BOTTOM 50
#define BORDER_RIGHT 20
#define BORDER_TOP 20

#define FONT GLUT_BITMAP_HELVETICA_10

// #define DEBUG_DISABLE_TIMER

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
    void draw_decorations(void);
    void draw_graphs(void);

    void draw_text_at_pixels(double x, double y, string s, hanchor_t halign, vanchor_t valign);

    void set_domain_pixels(void);
    void set_domain_device(void);
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
#ifndef DEBUG_DISABLE_TIMER
    Fl::add_timeout(1.0/FRAME_RATE_Hz, timer_cb, (void *)this);
#endif
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

void Pivotter::set_domain_pixels(void)
{
    glLoadIdentity();
    glViewport(0, 0, w(), h());
    glOrtho(0, w(), 0, h(), -1, 1);
}

void Pivotter::set_domain_device(void)
{
    glLoadIdentity();
    glViewport(BORDER_LEFT, BORDER_BOTTOM, w()-BORDER_LEFT-BORDER_RIGHT, h()-BORDER_TOP-BORDER_BOTTOM);
    glOrtho(xmin, xmax, ymin, ymax, -1, 1);
}

void Pivotter::draw()
{
    // Clear screen
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    draw_decorations();
    draw_graphs();

    return;
}

void Pivotter::draw_graphs(void)
{
    glPushMatrix();
        set_domain_device();

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

    glPopMatrix();
}

void Pivotter::draw_decorations(void)
{
    // Draw box axes
    glPushMatrix();
        set_domain_pixels();

        double bottom = BORDER_BOTTOM;
        double top = h()-BORDER_TOP;
        double left = BORDER_LEFT;
        double right = w()-BORDER_RIGHT;

        glLineWidth(1.0);
        glColor3ub(0x00, 0x00, 0x00);
        glBegin(GL_LINE_LOOP);
            glVertex2f(left, bottom);
            glVertex2f(left, top);
            glVertex2f(right, top);
            glVertex2f(right, bottom);
        glEnd();
        glBegin(GL_LINE_LOOP);
            glVertex2f((left+right)/2.0, bottom);
            glVertex2f((left+right)/2.0, top);
        glEnd();
        glBegin(GL_LINE_LOOP);
            glVertex2f(left, (bottom+top)/2.0);
            glVertex2f(right, (bottom+top)/2.0);
        glEnd();
    glPopMatrix();

    // Draw text labels
    const char *fmt = "%.3g";
    char s[MAX_LABEL_SIZE];

    sprintf(s, fmt, ymin);
    draw_text_at_pixels(BORDER_LEFT, BORDER_BOTTOM, string(s), HANCHOR_RIGHT, VANCHOR_BOTTOM);
    sprintf(s, fmt, (ymin+ymax)/2.0);
    draw_text_at_pixels(BORDER_LEFT, h()/2.0 - (BORDER_TOP-BORDER_BOTTOM)/2, string(s), HANCHOR_RIGHT, VANCHOR_CENTER);
    sprintf(s, fmt, ymax);
    draw_text_at_pixels(BORDER_LEFT, h()-BORDER_TOP, string(s), HANCHOR_RIGHT, VANCHOR_TOP);

    sprintf(s, fmt, xmin);
    draw_text_at_pixels(BORDER_LEFT, BORDER_BOTTOM, string(s), HANCHOR_LEFT, VANCHOR_TOP);
    sprintf(s, fmt, (xmin+xmax)/2.0);
    draw_text_at_pixels(w()/2.0 - (BORDER_RIGHT-BORDER_LEFT)/2, BORDER_BOTTOM, string(s), HANCHOR_CENTER, VANCHOR_TOP);
    sprintf(s, fmt, xmax);
    draw_text_at_pixels(w()-BORDER_RIGHT, BORDER_BOTTOM, string(s), HANCHOR_RIGHT, VANCHOR_TOP);

    return;

}

void Pivotter::draw_text_at_pixels(double x, double y, string s, hanchor_t halign, vanchor_t valign)
{
    double s_width = glutBitmapLength(FONT, (const unsigned char *)s.c_str());
    double s_height = glutBitmapHeight(FONT);

    double x_offset;
    double y_offset;

    switch (halign) {
    case HANCHOR_LEFT:
        x_offset = 0;
        break;
    case HANCHOR_CENTER:
        x_offset = s_width/2.0;
        break;
    case HANCHOR_RIGHT:
        x_offset = s_width;
        break;
    }

    switch (valign) {
    case VANCHOR_BOTTOM:
        y_offset = 0;
        break;
    case VANCHOR_CENTER:
        y_offset = s_height/2.0;
        break;
    case VANCHOR_TOP:
        y_offset = s_height;
        break;
    }

    x_offset *= 2.0;

    glPushMatrix();
        set_domain_pixels();

        glColor3ub(0x00, 0x00, 0x00);
        glRasterPos2f(x-x_offset, y-y_offset);
        for (unsigned int n = 0; n < s.length(); n++) {
            glutBitmapCharacter(FONT, s[n]);
        }
    glPopMatrix();
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
    Fl_Window win(800, 600, "Pivotter");
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
