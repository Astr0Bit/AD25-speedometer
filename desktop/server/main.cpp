#include <QThread>
#include "window.h"

#ifdef UARTCOM
#include "uartservice.h"
#else
#include "tcpservice.h"
#endif

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // * Set font
    QFont defaultFont("Arial", 10, QFont::Normal);
    app.setFont(defaultFont);
#ifdef UARTCOM
    UARTService service;
    if (!service.isRunning())
    {
        qCritical() << "Failed to create service, exiting...";
        std::exit(EXIT_FAILURE);
    }
#else
    TCPService service;
    if (!service.isRunning())
    {
        qCritical() << "Failed to create service, exiting...";
        std::exit(EXIT_FAILURE);
    }
#endif

    qDebug() << "Starting server GUI...";

    Window window(service);
    window.show();

    return app.exec();
}
