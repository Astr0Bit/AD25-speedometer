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
#include <string>
#include <tuple>
#include <cstdint>
#include <cassert>

namespace Setting
{
    class Signal
    {
        struct Info
        {
            uint32_t length;
            uint32_t start;
            int32_t min;
            int32_t max;
        };
        using signals_map_t = std::map<std::string_view, Info>;

        const signals_map_t m_signal_map;

        Signal() : m_signal_map{build_map()}
        {
        }

        static signals_map_t build_map()
        {
            signals_map_t map;
            const std::tuple<Info, std::string_view> signals_tuple[] SIGNALS;
            for (size_t i = 0; i < sizeof(signals_tuple) / sizeof(signals_tuple[0]); ++i)
            {
                std::string_view key = std::get<std::string_view>(signals_tuple[i]);
                const Info &val = std::get<Info>(signals_tuple[i]);
                assert(SBUFLEN >= (((val.start + val.length + 7) & ~7) >> 3));
                map[key] = val;
            }
            return map;
        }

    public:
        static const Signal &handle()
        {
            static Signal instance;
            return instance;
        }

        const Info &operator[](std::string_view key) const
        {
            return m_signal_map.at(key);
        }
    };

    constexpr int INTERVAL{40};

#ifdef UARTCOM
    namespace UART
    {
    }
#else
    namespace TCP
    {
        // Constants
        constexpr const int N_CONNS{1};
        constexpr const int RW_INTERVAL_MS{20};
        constexpr const char *IP{"127.0.0.1"};
        constexpr const int PORT{1337};
    }
#endif
} // namespace Setting
#endif

#endif // SETTING_H
