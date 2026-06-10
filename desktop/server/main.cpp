#include "window.h"
#include <QApplication>

// * Just for testing
#include <iostream>
class TestCom : public COMService
{
public:
    TestCom() { m_is_running = true; }
    bool sendBuffer() override { return true; }
    bool getStatus() const override { return true; }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // * Just for testing
    TestCom test;

    std::cout << "Initial: ";
    test.printBuffer();

    test.insertSpeed(240); // Max speed (All 1s for 8 bits)
    std::cout << "Speed 240: ";
    test.printBuffer();

    test.insertTemp(-60); // Negative temp test
    std::cout << "Temp -60: ";
    test.printBuffer();

    test.insertLightSignals(true, false);
    std::cout << "Left Light On: ";
    test.printBuffer();

    return app.exec();
}
