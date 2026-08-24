#ifndef UARTCOM_H
#define UARTCOM_H

#include <QThread>
#include <QSerialPort>
#include "comservice.h"

class UARTService : public QThread,
                    public COMService
{
    Q_OBJECT

public:
    // Constructor to begin sending data over UART
    explicit UARTService(QObject *parent = nullptr);

    // Helper method to get running status
    bool isRunning()
    {
        return m_isRunning.load();
    }

    // Destructor to clean up
    ~UARTService();

signals:
    // Notifies the Qt main thread if an error occured
    void fatalErrorOccurred(const QString &reason);

private:
    std::atomic_bool m_isRunning{true};

    // Constants
    const QString m_portName{UART_PORT};

    void run(void) override;
};

#endif