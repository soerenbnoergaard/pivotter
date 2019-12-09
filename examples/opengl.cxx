// Good examples:
// [1] http://seriss.com/people/erco/fltk/#OpenGlSimple
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

class Pivotter : public Fl_Gl_Window {
    // Draw method
    // OpenGL window: (w,h) is upper right, (-w,-h) is lower left, (0,0) is center

private:
    double xmin = 0.0;
    double xmax = 100.0;
    double ymin = 0.0;
    double ymax = 100.0;

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
        // Viewport not valid? Init viewport, ortho, etc.
        if (!valid()) {
            glLoadIdentity();
            glViewport(0,0, w(), h());
            glOrtho(-w(), w(), -h(), h(), -1, 1);
        }

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw something
        glColor3f(1.0, 0.0, 0.0);
        glBegin(GL_LINE_STRIP);
        glVertex2f(xscale(0.0), yscale(0.0));
        glVertex2f(xscale(50.0), yscale(100.0));
        glVertex2f(xscale(100.0), yscale(0.0));
        // glVertex2f(-w(), h());
        // glVertex2f(0, -h());
        // glVertex2f(w(), h());
        glEnd();
    }
public:
    // Constructor
    Pivotter(int X, int Y, int W, int H, const char*L=0) : Fl_Gl_Window(X, Y, W, H, L)
    {
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