#include <QDebug>
#include "setting.h"
#include <QSerialPort>
#include "uartservice.h"

// Public
UARTService::UARTService(QObject *parent)
{
    (void)parent;

    // Configure serial
    m_serial.setPortName(m_portName);
    qDebug() << "Set port name to" << m_portName;

    m_serial.setBaudRate(BAUDRATE);
    qDebug() << "Set baud rate to" << BAUDRATE;

    if (!m_serial.setDataBits(QSerialPort::Data8))
    {
        qCritical() << "Falied to set data bits for port!";
        m_isRunning.store(false);
        return;
    }
    if (!m_serial.setParity(QSerialPort::NoParity))
    {
        qCritical() << "Failed to set parity for port!";
        m_isRunning.store(false);
        return;
    }
    if (!m_serial.setStopBits(QSerialPort::OneStop))
    {
        qCritical() << "Failed to set stop bit for port!";
        m_isRunning.store(false);
        return;
    }

    if (!m_serial.setFlowControl(QSerialPort::NoFlowControl))
    {
        qCritical() << "Failed to set flow control for port!";
        m_isRunning.store(false);
        return;
    }

    qDebug() << "Successfully setup serial port...";

    m_serial.open(QIODeviceBase::WriteOnly);
    if (m_serial.isOpen())
    {
        m_status.store(true);

        // UARTService::run() to this thread
        m_serial.moveToThread(this);

        // Execute UARTService::run()
        this->start();
    }
    else
    {
        qDebug() << "Serial port is NOT connected. |" << m_serial.error();
        m_isRunning.store(false);
    }
}

UARTService::~UARTService()
{
    m_isRunning.store(false);
    this->wait();
}

// Private
void UARTService::run()
{
    // Outer loop
    while (true)
    {
        if (m_serial.isOpen())
        {
            qDebug() << "Serial port is connected. |" << m_serial.error();

            // Send whilst connected, and GUI is open
            while (m_isRunning)
            {
                // * Periodically send the COMService buffer

                qint64 bytes_sent = 0;

                // Temporary buffer
                uint8_t temp_buf[SBUFLEN]{0};
                {
                    std::lock_guard<std::mutex> lock(m_mtx);
                    memcpy(temp_buf, m_buf, sizeof(uint8_t) * SBUFLEN);
                }

                // Send the buffer
                bytes_sent = m_serial.write(reinterpret_cast<const char *>(temp_buf), SBUFLEN);

                bool write_success = m_serial.waitForBytesWritten(100);

                // Check for negative bytes, a write failure, or a flagged port error
                if (bytes_sent < 0 || !write_success || m_serial.error() != QSerialPort::NoError)
                {
                    // If the device was physically unplugged, the error usually flags as QSerialPort::ResourceError
                    qCritical() << "Connection lost or write failed. Error code:" << m_serial.error();
                    m_status.store(false);
                    break;
                }

                // Wait x milliseconds before sending the next packet
                QThread::msleep(Setting::INTERVAL);
            }

            // Close the serial port
            m_serial.close();
        }

        // Reconnection branch
        else
        {
            if (!m_isRunning)
            {
                break;
            }
            qDebug() << "Serial port is NOT connected. |" << m_serial.error();
            QThread::msleep(1000);
            m_serial.open(QIODeviceBase::WriteOnly);
        }
    }
}