#ifndef UARTCOM_H
#define UARTCOM_H

#include <QThread>
#include "comservice.h"

class UARTService : public QThread, public COMService
{
    Q_OBJECT
private:
    std::atomic_bool m_stop{false};
public:
    explicit UARTService(QObject* parent = nullptr);
    ~UARTService() override;

protected:
    void run() override;
};

#endif