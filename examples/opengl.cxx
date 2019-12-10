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
        glClear(GL_COLOR_BUFFER_BIT);

        // Plot all lines
        for (auto it = data.cbegin(); it != data.cend(); it++) {
            string hue = it->first;

            // Draw line
            glColor3f(1.0, 0.0, 0.0);
            glBegin(GL_LINE_STRIP);
            for (int n = 0; n < num_samples; n++) {
                glVertex2f(xscale(data[hue].x[n]), yscale(data[hue].y[n]));
            }
            glEnd();
        }
    }

    static void timer_cb(void *handle)
    {
        Pivotter *h = (Pivotter *)handle;
        double x, y;
        get_sample(&x, &y);
        h->add(x, y);
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

    void add(double x, double y)
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

        data["tmp"].x.push_back(x);
        data["tmp"].y.push_back(y);
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
