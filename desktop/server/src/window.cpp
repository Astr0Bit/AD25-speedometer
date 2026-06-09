#include "window.h"
#include "setting.h"

// * Resources used:
// - https://doc.qt.io/qt-6/qtwidgets-widgets-sliders-example.html
// - https://doc.qt.io/qt-6/qslider.html
// - https://zetcode.com/pyqt/qslider/

Window::Window()
{
    // TODO -> Get min, max values for each slider using Setting::Signal class

    // // * NOTE: Just for experimenting
    // sld_speed.setMinimum(0);
    // sld_speed.setMaximum(240);

    // * Add text before each slider
    layout.addWidget(&lbl_speed, 0, 0, 1, 1);
    layout.addWidget(&lbl_bat, 1, 0, 1, 1);
    layout.addWidget(&lbl_temp, 2, 0, 1, 1);
    layout.addWidget(&lbl_light, 3, 0, 1, 1);

    // * Add the sliders to the window
    layout.addWidget(&sld_speed, 0, 1, 1, 1);
    layout.addWidget(&sld_bat, 1, 1, 1, 1);
    layout.addWidget(&sld_temp, 2, 1, 1, 1);

    // TODO -> Add text after each slider

    // * Add the checkboxes
    layout.addWidget(&box_left, 3, 1, 1, 1);
    layout.addWidget(&box_right, 3, 2, 1, 1);
    layout.addWidget(&box_warning, 3, 3, 1, 1);

    setLayout(&layout);

    setWindowTitle("Server");

    resize(500, 200);
}