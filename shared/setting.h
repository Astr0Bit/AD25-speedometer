#ifndef SETTING_H
#define SETTING_H

#ifdef UARTCOM
#define BAUDRATE 1048576
#endif
#define SBUFLEN 3 // The required number of bytes to pack the signals

#ifdef __cplusplus
#define SIGNALS {                       \
    {{8, 0, 0, 240}, "speed"},          \
    {{7, 8, -60, 60}, "temperature"},   \
    {{7, 15, 0, 100}, "battery_level"}, \
    {{1, 22, 0, 1}, "left_light"},      \
    {{1, 23, 0, 1}, "right_light"},     \
}

#include <map>
#include <tuple>
#include <string>

namespace Setting
{
    class Signal
    {
    };

    constexpr int INTERVAL{40};

#ifdef UARTCOM
    namespace UART
    {
    }
#else
    namespace TCP
    {
    }
#endif
}
#endif

#endif // SETTING_H
