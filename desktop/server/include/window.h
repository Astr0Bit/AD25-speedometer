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
    // * Use grid layout
    QGridLayout layout;

    // * Window elements
    QSlider sld_bat{Qt::Horizontal};
    QSlider sld_temp{Qt::Horizontal};
    QSlider sld_speed{Qt::Horizontal};

public:
    // * Window and element placement created here
    Window();
};

#endif