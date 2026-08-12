#include "window.h"
#include "setting.h"

Window::Window(QWidget *parent)
    : QDialog(parent),
      canvas_(this),
      layout_(this)
{
    setWindowTitle("Speedometer client");

    // Test values until the GUI is connected to COMService.
    canvas_.setSpeed(110);
    canvas_.setTemperature(30);
    canvas_.setBatteryLevel(100);
    canvas_.setCommunicationStatus(true);
    canvas_.setLightSignals(false, false, false);

    layout_.setContentsMargins(0, 0, 0, 0);
    layout_.addWidget(&canvas_, 0, 0);

    connect(&blinkTimer_, &QTimer::timeout, this, [this]()
            {
        static bool visible = false;
        visible = !visible;
        canvas_.setBlinkVisible(visible); });
    blinkTimer_.start(canvas_.m_blink_interval_ms);
}
