// Good examples:
// [1] http://seriss.com/people/erco/fltk/#OpenGlSimple
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

#include <vector>
#include <unordered_map>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

#define FRAME_RATE_Hz 10
#define MAX_NUM_HUES 10

GLubyte color_lut[MAX_NUM_HUES][3] = {
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
} xy_t;

typedef unordered_map<string, xy_t> data_t;

void get_sample(double *x, double *y)
{
    *x = rand();
    *y = rand();
}

class Pivotter : public Fl_Gl_Window {
private:
    data_t data;
    int num_samples = 0;
    double xmin;
    double xmax;
    double ymin;
    double ymax;

    GLfloat xscale(double x)
    {
        return -w() + (x-xmin)*(w()+w())/(xmax-xmin);
    }

    GLfloat yscale(double y)
    {
        return -h() + (y-ymin)*(h()+h())/(ymax-ymin);
    }

    void draw()
    {
        if (!valid()) {
            glLoadIdentity();
            glViewport(0,0, w(), h());
            glOrtho(-w(), w(), -h(), h(), -1, 1);
        }

        // Clear screen
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Plot all lines
        int i = 0;
        for (auto it = data.cbegin(); it != data.cend(); it++) {
            if (MAX_NUM_HUES-1 < i) {
                continue;
            }

            string hue = it->first;

            // Draw line
            glColor3ub(color_lut[i][0], color_lut[i][1], color_lut[i][2]);
            glBegin(GL_LINE_STRIP);
            for (int n = 0; n < num_samples; n++) {
                glVertex2f(xscale(data[hue].x[n]), yscale(data[hue].y[n]));
            }
            glEnd();

            i += 1;
        }
    }

    static void timer_cb(void *handle)
    {
        Pivotter *h = (Pivotter *)handle;
        double x, y;
        get_sample(&x, &y);
        h->add(x, y, "tmp");
        h->redraw();
        Fl::repeat_timeout(1.0/FRAME_RATE_Hz, timer_cb, handle);
    }
public:
    // Constructor
    Pivotter(int X, int Y, int W, int H, const char*L=0) : Fl_Gl_Window(X, Y, W, H, L)
    {
        reset();
        Fl::add_timeout(1.0/FRAME_RATE_Hz, timer_cb, (void *)this);
        end();
    }

    void reset(void)
    {
        data.clear();
        num_samples = 0;
        xmin = 0.0;
        xmax = 0.0;
        ymin = 0.0;
        ymax = 0.0;
    }

    void add(double x, double y, string hue)
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

        data[hue].x.push_back(x);
        data[hue].y.push_back(y);
        num_samples += 1;

        draw();
    }
};

int main()
{
    Fl_Window win(500, 300, "Pivotter");
    Pivotter pivotter(10, 10, win.w()-20, win.h()-20);
    win.end();
    win.resizable(pivotter);
    win.show();

    return(Fl::run());
}
