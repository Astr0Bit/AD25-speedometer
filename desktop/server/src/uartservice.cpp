#include <QDebug>
#include "setting.h"
#include <QSerialPort>
#include "uartservice.h"
#include <QCoreApplication>

// Public
UARTService::UARTService(QObject *parent) : QThread(parent)
{
    // Connects the fatalErrorOccurred signal to the main Qt application
    // This makes the termination process graceful, and does not use std::exit
    connect(this, &UARTService::fatalErrorOccurred, qApp, [](const QString &reason)
            {
        qCritical() << "Terminating application due to UART failure:" << reason;
        QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);

    this->start();
}

UARTService::~UARTService()
{
    m_isRunning.store(false);
    this->wait();
}

// Private
void UARTService::run()
{
    // Configure serial
    QSerialPort serial;
    serial.setPortName(m_portName);
    qDebug() << "Set port name to" << m_portName;

    serial.setBaudRate(BAUDRATE);
    qDebug() << "Set baud rate to" << BAUDRATE;

    if (!serial.setDataBits(QSerialPort::Data8))
    {
        emit fatalErrorOccurred("Falied to set data bits for port!");
        return;
    }
    if (!serial.setParity(QSerialPort::NoParity))
    {
        emit fatalErrorOccurred("Failed to set parity for port!");
        return;
    }
    if (!serial.setStopBits(QSerialPort::OneStop))
    {
        emit fatalErrorOccurred("Failed to set stop bit for port!");
        return;
    }

    if (serial.setFlowControl(QSerialPort::NoFlowControl))
    {
        emit fatalErrorOccurred("Failed to set flow control for port!");
        return;
    }

    // Open the port
    serial.open(QIODeviceBase::WriteOnly);

    // Outer loop
    while (m_isRunning)
    {
        if (serial.isOpen())
        {
            qDebug() << "Serial port is connected. |" << serial.error();
            m_status.store(true);

            // Send whilst connected, and GUI is open
            while (m_isRunning && m_status)
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
                bytes_sent = serial.write(reinterpret_cast<const char *>(temp_buf), SBUFLEN);

                bool write_success = serial.waitForBytesWritten(100);

                // Check for negative bytes, a write failure, or a flagged port error
                if (bytes_sent < 0 || !write_success || serial.error() != QSerialPort::NoError)
                {
                    // If the device was physically unplugged, the error usually flags as QSerialPort::ResourceError
                    qCritical() << "Connection lost or write failed. Error code:" << serial.error();
                    m_status.store(false);
                    break;
                }

                // Wait x milliseconds before sending the next packet
                QThread::msleep(Setting::INTERVAL);
            }

            // Close the serial port
            serial.close();
        }

        // Reconnection branch
        else
        {
            qDebug() << "Serial port is NOT connected. |" << serial.error();
            QThread::msleep(1000);
            serial.open(QIODeviceBase::WriteOnly);
        }
    }
}