#include "window.h"
#include "tcpservice.h"
#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // * Set font
    QFont defaultFont("Arial", 10, QFont::Normal);
    app.setFont(defaultFont);

    // * NOTE -> This should be self-determined
    TCPService tcp_service;

    Window window(tcp_service);
    window.show();

    // Start the TCP/IP service
    tcp_service.run();

    return app.exec();
}
