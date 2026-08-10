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

    return app.exec();
}
