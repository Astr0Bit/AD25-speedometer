#include "window.h"
#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // * Set font
    QFont defaultFont("Arial", 10, QFont::Normal);
    app.setFont(defaultFont);

    Window window;
    window.show();

    return app.exec();
}
