#include <QDebug>
#include <QSerialPort>
#include <cstring>
#include "setting.h"
#include "uartservice.h"

UARTService::UARTService(QObject* parent) : COMService{}, QThread{parent}
{
    this->start();
}

UARTService::~UARTService()
{
    m_stop = true;
    this->wait();
}

void UARTService::run()
{
    QSerialPort serial;
    serial.setPortName(QString{UART_PORT});
    serial.setBaudRate(BAUDRATE);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    serial.setReadBufferSize(SBUFLEN);

    uint8_t tmpbuf[SBUFLEN];
    while (!m_stop)
    {
        if (!m_status)
        {
            qDebug() << "Trying to open serialport...  PORT_NAME:" << UART_PORT << " BAUDRATE:" << BAUDRATE;
            if (!serial.open(QIODevice::ReadOnly))
            {
                qWarning() << "  Failed to open serialport: " << serial.error();
                this->msleep(Setting::UART::CLIENT_RETRY_OPEN_INTERVAL);
                continue;
            }
            qDebug() << "Opened serialport";
            serial.clear();
        }

        if (serial.waitForReadyRead(3 * Setting::INTERVAL))
        {
            if (SBUFLEN == serial.read(reinterpret_cast<char*>(tmpbuf), SBUFLEN))
            {
                {
                    std::lock_guard<std::mutex> lock{m_mtx};
                    std::memcpy(m_buffer, tmpbuf, SBUFLEN);
                }
                m_status = true;
            }
            else
            {
                m_status = false;
                serial.close();
                qWarning() << "Failed to read: " << serial.error();
                this->msleep(Setting::UART::CLIENT_RETRY_OPEN_INTERVAL);
            }
        }
        else
        {
            m_status = false;
            serial.close();
            qWarning() << "Failed waiting for ready read: " << serial.error();
            this->msleep(Setting::UART::CLIENT_RETRY_OPEN_INTERVAL);
        }
    }

    serial.close();
}