#include "window.h"
#include "setting.h"

// * Resources used:
// - https://doc.qt.io/qt-6/qtwidgets-widgets-sliders-example.html
// - https://doc.qt.io/qt-6/qslider.html

Window::Window()
{
    // TODO -> Get min, max values for each slider using Setting::Signal class

    // * NOTE: Just for experimenting
    sld_speed.setMinimum(0);
    sld_speed.setMaximum(240);

    // * Add the sliders to the window
    layout.addWidget(&sld_speed, 0, 0, 1, 1);
    layout.addWidget(&sld_bat, 1, 0, 1, 1);
    layout.addWidget(&sld_temp, 2, 0, 1, 1);

    setLayout(&layout);

    setWindowTitle("Server");
}