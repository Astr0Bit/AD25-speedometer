#include "window.h"
#include "setting.h"

// * Resources used:
// - https://doc.qt.io/qt-6/qtwidgets-widgets-sliders-example.html
// - https://doc.qt.io/qt-6/qslider.html
// - https://zetcode.com/pyqt/qslider/
// - https://doc.qt.io/qt-6/qcheckbox.html
// - https://stackoverflow.com/questions/61692602/how-can-i-make-the-qslider-indicate-the-current-value
// - https://doc.qt.io/qt-6/qhboxlayout.html
// - https://doc.qt.io/qt-6/layout.html
// - https://stackoverflow.com/questions/24016264/qt-how-to-disable-qcheckbox-while-retaining-checked-state
// - https://doc.qt.io/qt-6/qabstractbutton.html#checked-prop

Window::Window()
{
    // * NOTE: Could be improved with functions

    // * Add text before each slider
    sld_layout.addWidget(&lbl_speed, 0, 0, 1, 1);
    sld_layout.addWidget(&lbl_temp, 1, 0, 1, 1);
    sld_layout.addWidget(&lbl_bat, 2, 0, 1, 1);
    sld_layout.addWidget(&lbl_light, 3, 0, 1, 1);

    // * Add the sliders to the window
    sld_layout.addWidget(&sld_speed, 0, 1, 1, 1);
    sld_layout.addWidget(&sld_temp, 1, 1, 1, 1);
    sld_layout.addWidget(&sld_bat, 2, 1, 1, 1);

    // * Add text after each slider
    sld_layout.addWidget(&lbl_speed_val, 0, 2, 1, 1);
    sld_layout.addWidget(&lbl_temp_val, 1, 2, 1, 1);
    sld_layout.addWidget(&lbl_bat_val, 2, 2, 1, 1);

    // * Add methods to the sliders
    connect(&sld_speed, &QSlider::valueChanged, [this](int value)
            { lbl_speed_val.setText(QString("%1 Kph").arg(value)); });

    connect(&sld_bat, &QSlider::valueChanged, [this](int value)
            { lbl_bat_val.setText(QString("%1 %").arg(value)); });

    connect(&sld_temp, &QSlider::valueChanged, [this](int value)
            { lbl_temp_val.setText(QString("%1 °C").arg(value)); });

    // * Put the checkboxes within a separate layout
    // * Add the checkboxes
    box_layout.addWidget(&box_left);
    box_layout.addWidget(&box_right);
    box_layout.addWidget(&box_warning);

    // * So the left checkbox becomes disabled if right is clicked, and vice versa
    connect(&box_left, &QAbstractButton::toggled, [this](bool checked)
            { box_right.setEnabled(!checked); });

    connect(&box_right, &QAbstractButton::toggled, [this](bool checked)
            { box_left.setEnabled(!checked); });

    // TODO -> Get min, max values for each slider using Setting::Signal class
    // * NOTE: Just for experimenting
    sld_speed.setMinimum(0);
    sld_speed.setMaximum(240);
    sld_speed.setValue(0);

    sld_bat.setMinimum(0);
    sld_bat.setMaximum(100);
    sld_bat.setValue(0);

    sld_temp.setMinimum(-60);
    sld_temp.setMaximum(60);
    sld_temp.setValue(-60);

    // * Link the box layout to the slider layout
    sld_layout.addLayout(&box_layout, 3, 1);

    setLayout(&sld_layout);

    setWindowTitle("Server");

    resize(700, 200);
}