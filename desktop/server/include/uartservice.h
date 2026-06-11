#ifndef UARTCOM_H
#define UARTCOM_H

#include <QThread>
#include "comservice.h"

class UARTService : public COMService, public QThread
{
    // * Just for testing
    void run(void) override {};
};

#endif