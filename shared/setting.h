#ifndef SETTING_H
#define SETTING_H

#ifdef UARTCOM
#define BAUDRATE 1048576
#endif
#define SBUFLEN 3 // The required number of bytes to pack the signals

#ifdef __cplusplus
#define NUM_SIGNALS 5
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
    public:
        static const Signal& handle()
        {
            static Signal instance;
            return instance;
        }
        struct Info {
            int length;
            int start;
            int min;
            int max;
        };

        const Info& operator[](const std::string& key) const
        {
            return std::get<Info>(m_signals[m_signal_map.at(key)]);
        }

    private:
        Signal(const Signal&) = delete;
        Signal& operator=(const Signal&) = delete;
        Signal(Signal&&) = delete;
        Signal& operator=(Signal&&) = delete;
        Signal() :
            m_signals SIGNALS,
            m_signal_map{
                {std::get<std::string>(m_signals[0]), 0},
                {std::get<std::string>(m_signals[1]), 1},
                {std::get<std::string>(m_signals[2]), 2},
                {std::get<std::string>(m_signals[3]), 3},
                {std::get<std::string>(m_signals[4]), 4},
            } {
        }

        const std::tuple<Info, std::string> m_signals[NUM_SIGNALS];
        const std::map<std::string, int> m_signal_map;
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
