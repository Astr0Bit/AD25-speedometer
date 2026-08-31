#include "window.h"
#include "setting.h"

Window::Window(COMService &com_service, QWidget *parent)
    : QDialog(parent),
      m_canvas(this),
      m_layout(this),
      m_com_service(com_service)
{
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setWindowTitle("Speedometer client");

    m_layout.setContentsMargins(0, 0, 0, 0);
    m_layout.addWidget(&m_canvas, 0, 0);

    connect(&m_blinkTimer, &QTimer::timeout, this, [this]()
            {
        static bool visible = false;
        visible = !visible;
        m_canvas.setBlinkVisible(visible); });
    m_blinkTimer.start(m_canvas.m_blink_interval_ms);

    // Timer which both:
    // - checks connection (com_service.m_status)
    // - periodically reads the buffer
    connect(&m_connBufTimer, &QTimer::timeout, this, [this]()
            {
        // Check connection
        if (m_com_service.getStatus()) {
            // Set connection status
            m_canvas.setCommunicationStatus(true);

            // Read buffer
            static int8_t temp{0};
            static uint8_t speed{0};
            static uint8_t battery{0};
            static bool left_light{false};
            static bool right_light{false};

            // Read and set speed
            m_com_service.getSpeed(speed);
            m_canvas.setSpeed(speed);

            // Read and set temp
            m_com_service.getTemp(temp);
            m_canvas.setTemperature(temp);

            // Read and set battery
            m_com_service.getBattery(battery);
            m_canvas.setBatteryLevel(battery);

            // Read and set light signals
            m_com_service.getLightSignals(left_light, right_light);
            m_canvas.setLightSignals(left_light, right_light);
        }
        else {
            m_canvas.setCommunicationStatus(false);
            m_com_service.clearBuffer();
        } });

    m_connBufTimer.start(Setting::INTERVAL);
}
