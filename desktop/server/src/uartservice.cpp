#include <QDebug>
#include "setting.h"
#include <QSerialPort>
#include "uartservice.h"
#include <QCoreApplication>

// Public
UARTService::UARTService(QObject *parent) : QThread(parent)
{
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

    qDebug() << "Begin serial port config...";
    // ! NOTE: These can fail, but only if the port is opened before, which it isn't
    serial.setPortName(m_portName);
    serial.setBaudRate(BAUDRATE);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    qDebug() << "Finished serial port config";

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