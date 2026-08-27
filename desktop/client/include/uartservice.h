#ifndef UARTCOM_H
#define UARTCOM_H

#include <QThread>
#include "comservice.h"

class UARTService : public COMService, public QThread
{
    std::atomic_bool m_stop{false};

    void run() override;
public:
    explicit UARTService(QObject* parent = nullptr);
    ~UARTService();
};

#endif