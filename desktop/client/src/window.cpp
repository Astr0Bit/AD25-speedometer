#include "window.h"
#include "setting.h"

Window::Window(QWidget *parent)
    : QDialog(parent),
      m_canvas(this),
      m_layout(this)
{
    setWindowTitle("Speedometer client");

    // Test values until the GUI is connected to COMService.
    m_canvas.setSpeed(110);
    m_canvas.setTemperature(30);
    m_canvas.setBatteryLevel(100);
    m_canvas.setCommunicationStatus(true);
    m_canvas.setLightSignals(false, false, false);

    m_layout.setContentsMargins(0, 0, 0, 0);
    m_layout.addWidget(&m_canvas, 0, 0);

    connect(&m_blinkTimer, &QTimer::timeout, this, [this]()
            {
        static bool visible = false;
        visible = !visible;
        m_canvas.setBlinkVisible(visible); });
    m_blinkTimer.start(m_canvas.m_blink_interval_ms);
}
