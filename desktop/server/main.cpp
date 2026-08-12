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
    #else
    TCPService service;
    #endif

    Window window(service);
    window.show();

    return app.exec();
}
