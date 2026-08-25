#include "window.h"
#include "setting.h"
#include "comservice.h"

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

/**
 * @brief Helper function to make the grid
 *
 * @tparam COLS Number of columns for the grid
 * @tparam ROWS Number of rows for the grid
 * @param grid_layout Array with columns and rows
 */
template <size_t COLS, size_t ROWS>
static void make_grid(QGridLayout &grid_layout, QWidget *(&cols)[COLS][ROWS])
{
    // Set grid spacing
    grid_layout.setVerticalSpacing(0);
    grid_layout.setContentsMargins(4, 0, 4, 0);

    for (size_t col = 0; col < COLS; ++col)
    {
        for (size_t row = 0; row < ROWS; ++row)
        {
            QWidget *widget = cols[col][row];
            if (!widget)
                continue;

            // Set fixed width to the labels
            if (QLabel *label = dynamic_cast<QLabel *>(widget))
            {
                // Make labels wider
                if (col == (COLS - 1))
                {
                    label->setIndent(5);
                    label->setFixedWidth(70);
                }
                else
                {
                    label->setIndent(15);
                    label->setFixedWidth(120);
                }
            }

            grid_layout.addWidget(widget, static_cast<int>(row), static_cast<int>(col), 1, 1);
        }
    }
}

Window::Window(COMService &com_service) : m_com_service(com_service)
{
    // Makes the GUI "sticky", ie. not minimize when the user clicks outside it
    // NOTE: Works only with X11 display server, not Wayland
    this->setWindowFlags(Qt::WindowStaysOnTopHint);

    // * NOTE: Could be improved with functions

    // Array with all elements
    QWidget *cols[3][4] = {
        // Text before each slider
        {&lbl_speed, &lbl_temp, &lbl_bat, &lbl_light},

        // Sliders
        {&sld_speed, &sld_temp, &sld_bat, nullptr},

        // Text after each slider
        {&lbl_speed_val, &lbl_temp_val, &lbl_bat_val, nullptr},
    };
    make_grid(sld_layout, cols);

    // Lambda to make adding methods to sliders more DRY
    auto setupSlider = [this](QSlider &slider, QLabel &valLabel, const QString &unit, int min, int max, auto setterMethod)
    {
        slider.setRange(min, max);
        slider.setValue(min);

        // Set initial text of label to min value
        valLabel.setText(QString("%1 %2").arg(min).arg(unit));

        // Connect the slider to update the label dynamically
        connect(&slider, &QSlider::valueChanged, this, [&valLabel, unit, this, setterMethod](int value)
                {
                    // Set text of label
                    valLabel.setText(QString("%1 %2").arg(value).arg(unit));

                    // Invoke the COMService setter method
                    (m_com_service.*setterMethod)(value); });

        // Add min value for each signal to the buffer on boot
        (m_com_service.*setterMethod)(min);
    };

    // Get min, max values for each slider using Setting::Signal class
    const auto &speed_info = Setting::Signal::handle()["speed"];
    const auto &temp_info = Setting::Signal::handle()["temperature"];
    const auto &bat_info = Setting::Signal::handle()["battery_level"];

    // Add methods to the sliders
    setupSlider(sld_bat, lbl_bat_val, "%", bat_info.min, bat_info.max, &COMService::setBattery);
    setupSlider(sld_temp, lbl_temp_val, "°C", temp_info.min, temp_info.max, &COMService::setTemp);
    setupSlider(sld_speed, lbl_speed_val, "Kph", speed_info.min, speed_info.max, &COMService::setSpeed);

    // Put the checkboxes within a separate layout
    // Add the checkboxes
    box_layout.addWidget(&box_left);
    box_layout.addWidget(&box_right);
    box_layout.addWidget(&box_warning);

    // So the left checkbox becomes disabled if right is clicked, and vice versa
    connect(&box_left, &QAbstractButton::toggled, [this](bool checked)
            { box_right.setEnabled(!checked); });

    connect(&box_right, &QAbstractButton::toggled, [this](bool checked)
            { box_left.setEnabled(!checked); });

    // Connect the checkboxes, so they can update the buffer
    auto updateLights = [this]()
    {
        bool is_left_active = box_left.isChecked() || box_warning.isChecked();
        bool is_right_active = box_right.isChecked() || box_warning.isChecked();

        // Push values to buffer
        m_com_service.setLightSignals(is_left_active, is_right_active);
    };

    connect(&box_left, &QCheckBox::toggled, this, updateLights);
    connect(&box_right, &QCheckBox::toggled, this, updateLights);
    connect(&box_warning, &QCheckBox::toggled, this, updateLights);

    // Set initial light signal values in buffer
    updateLights();

    // Link the box layout to the slider layout
    sld_layout.addLayout(&box_layout, 3, 1);
    setLayout(&sld_layout);

    setWindowTitle("Server");
    resize(700, 200);
}