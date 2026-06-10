#include "window.h"
#include "setting.h"

Window::Window(QWidget *parent)
    : QDialog(parent),
      canvas_(new Canvas(this))
{
    setWindowTitle("Speedometer client");
    setFixedSize(840, 588);

    // Test values until the GUI is connected to COMService.
    canvas_->setSpeed(110);
    canvas_->setTemperature(30);
    canvas_->setBatteryLevel(100);
    canvas_->setLightSignals(false, false, false);
    canvas_->setCommunicationStatus(true, "Connected");

    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(canvas_, 0, 0);

    connect(&blinkTimer_, &QTimer::timeout, this, [this]()
            {
        static bool visible = false;
        visible = !visible;
        canvas_->setBlinkVisible(visible); });
    blinkTimer_.start(330);
}
