#ifndef WINDOW_H
#define WINDOW_H

#include <QLabel>
#include <QDialog>
#include <QSlider>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include "comservice.h"

class Window : public QDialog
{
private:
    // Use grid layout
    QGridLayout sld_layout;
    QHBoxLayout box_layout;

    // Window elements *
    // Labels
    QLabel lbl_speed{tr("Speed: ")};
    QLabel lbl_speed_val{tr("0 Kph")};

    QLabel lbl_bat{tr("Battery: ")};
    QLabel lbl_bat_val{tr("0 %")};

    QLabel lbl_temp{tr("Temperature: ")};
    QLabel lbl_temp_val{tr("0 °C")};

    QLabel lbl_light{tr("Light Signals: ")};

    // Sliders
    QSlider sld_bat{Qt::Horizontal};
    QSlider sld_temp{Qt::Horizontal};
    QSlider sld_speed{Qt::Horizontal};

    // Checkboxes
    QCheckBox box_left{"Left"};
    QCheckBox box_right{"Right"};
    QCheckBox box_warning{"Warning"};

    // COMService reference (singleton)
    COMService &m_com_service;

public:
    /**
     * @brief Construct a new Window object
     *        Window and element placement created here
     *
     * @param com_service The protocol to use for sending the buffer
     */
    Window(COMService &com_service);
};

#endif