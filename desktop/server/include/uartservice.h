#ifndef UARTCOM_H
#define UARTCOM_H

#include <QThread>
#include "comservice.h"

class UARTService : public QThread,
                    public COMService
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new UARTService object
     *        Starts the UARTService::run worker thread
     *
     * @param parent Optional parent
     */
    explicit UARTService(QObject *parent = nullptr);

    // Destructor to clean up
    ~UARTService();

private:
    std::atomic_bool m_isRunning{true};

    // Constants
    const QString m_portName{UART_PORT};

    void run(void) override;
};

#endif