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

bool UARTService::isRunning() const noexcept
{
    return !m_stop;
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
            m_status = true;
            qDebug() << "Opened serialport";
        }

        if (serial.waitForReadyRead(3 * Setting::INTERVAL))
        {
            size_t nread{0};
            bool ok{true};
            while (nread < SBUFLEN && !m_stop)
            {
                qint64 n = serial.read(reinterpret_cast<char*>(tmpbuf + nread), static_cast<qint64>(SBUFLEN - nread));
                if (n > 0)
                {
                    nread += static_cast<size_t>(n);
                }
                else if (n < 0)
                {
                    m_status = false;
                    ok = false;
                    serial.close();
                    qWarning() << "Failed to read: " << serial.error();
                    this->msleep(Setting::UART::CLIENT_RETRY_OPEN_INTERVAL);
                    break;
                }
                else
                {
                    if (!serial.waitForReadyRead(20))
                    {
                        ok = false;
                        qWarning() << "Not enough bytes received:  Expected:" << SBUFLEN << " Actual:" << nread;
                        break;
                    }
                }
            }
            if (ok)
            {
                {
                    std::lock_guard<std::mutex> lock{m_mtx};
                    std::memcpy(m_buffer, tmpbuf, SBUFLEN);
                }
                qDebug() << "Received: " << tmpbuf[0] << " " << tmpbuf[1] << " " << tmpbuf[2];
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