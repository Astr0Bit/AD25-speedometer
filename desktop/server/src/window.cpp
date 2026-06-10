#include "window.h"
#include "setting.h"
#include <vector>

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

// * Helper function to make the grid
void make_grid(QGridLayout &grid_layout, std::vector<std::vector<QWidget *>> &cols)
{
    for (size_t col = 0; col < cols.size(); ++col)
    {
        for (size_t row = 0; row < cols[col].size(); ++row)
        {
            grid_layout.addWidget(cols[col][row], static_cast<int>(row), col, 1, 1);
        }
    }
}

Window::Window()
{
    // * NOTE: Could be improved with functions

    // * Vector with all elements
    std::vector<std::vector<QWidget *>> cols = {
        // * Text before each slider
        {&lbl_speed, &lbl_temp, &lbl_bat, &lbl_light},

        // * Sliders
        {&sld_speed, &sld_temp, &sld_bat},

        // * Text after each slider
        {&lbl_speed_val, &lbl_temp_val, &lbl_bat_val},
    };
    make_grid(sld_layout, cols);

    // * Lambda to make adding methods to sliders more DRY
    auto setupSlider = [this](QSlider &slider, QLabel &valLabel, const QString &unit, int min, int max)
    {
        slider.setRange(min, max);
        slider.setValue(min);

        // Set initial text of label to min value
        valLabel.setText(QString("%1 %2").arg(min).arg(unit));

        // Connect the slider to update the label dynamically
        connect(&slider, &QSlider::valueChanged, [&valLabel, unit](int value)
                { valLabel.setText(QString("%1 %2").arg(value).arg(unit)); });
    };

    // * Get min, max values for each slider using Setting::Signal class
    const auto &speed_info = Setting::Signal::handle()["speed"];
    const auto &temp_info = Setting::Signal::handle()["temperature"];
    const auto &bat_info = Setting::Signal::handle()["battery_level"];

    // * Add methods to the sliders
    setupSlider(sld_bat, lbl_bat_val, "%", bat_info.min, bat_info.max);
    setupSlider(sld_temp, lbl_temp_val, "°C", temp_info.min, temp_info.max);
    setupSlider(sld_speed, lbl_speed_val, "Kph", speed_info.min, speed_info.max);

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

    // * Link the box layout to the slider layout
    sld_layout.addLayout(&box_layout, 3, 1);
    setLayout(&sld_layout);

    setWindowTitle("Server");
    resize(700, 200);
}