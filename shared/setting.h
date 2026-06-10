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

namespace Setting {
    class Signal {
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

        const Info& operator[](std::string_view key) const
        {
            return m_signal_map.at(key);
        }

    private:
        using signals_map_t = std::map<std::string_view, Info>;

        Signal(const Signal&) = delete;
        Signal& operator=(const Signal&) = delete;
        Signal(Signal&&) = delete;
        Signal& operator=(Signal&&) = delete;
        Signal() : m_signal_map{build_map()}
        {
        }

        static signals_map_t build_map()
        {
            signals_map_t map;
            const std::tuple<Info, std::string_view> signals_tuple[] SIGNALS;
            for (size_t i = 0; i < sizeof(signals_tuple) / sizeof(signals_tuple[0]); ++i)
            {
                map[std::get<std::string_view>(signals_tuple[i])] = std::get<Info>(signals_tuple[i]);
            }
            return map;
        }

        const signals_map_t m_signal_map;
    };

    constexpr int INTERVAL{40};

#ifdef UARTCOM
    namespace UART {
    }
#else
    namespace TCP {
    }
#endif
}  // namespace Setting
#endif

#endif  // SETTING_H
