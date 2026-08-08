#include <climits>
#include <cstring>
#include <cassert>
#include "comservice.h"

uint64_t COMService::extract64(size_t byte_off)
{
    std::unique_lock<std::mutex> lock{m_mtx, std::defer_lock};
    uint64_t v{};
    if (byte_off + 8 <= SBUFLEN)
    {
        lock.lock();
        std::memcpy(&v, m_buffer + byte_off, 8);
    }
    else
    {
        const size_t n{SBUFLEN - byte_off};
        lock.lock();
        for (size_t i = 0; i < n; ++i)
            v |= static_cast<uint64_t>(m_buffer[byte_off + i]) << (i << 3);
    }
    return v;
}
template <typename T>
T COMService::read(size_t bit_off, size_t bit_len)
{
    const size_t byte_off = bit_off >> 3;
    assert(byte_off <= SBUFLEN);
    const uint64_t mask = (bit_len >= 64) ? ~0ULL : ((1ULL << bit_len) - 1);
    uint64_t bits = (extract64(byte_off) >> (bit_off & 7)) & mask;
    if constexpr (std::is_signed_v<T>)
    {
        if (bit_len > 0 && bit_len < 64)
        {
            const uint64_t sbit = 1ULL << (bit_len - 1);
            bits = (bits ^ sbit) - sbit;
        }
    }
    return static_cast<T>(bits);
}

void COMService::getSpeed(uint8_t& out)
{
    const auto& si = Setting::Signal::handle()["speed"];
    out = read<uint8_t>(si.start, si.length);
}
void COMService::getTemp(int8_t& out)
{
    const auto& si = Setting::Signal::handle()["temperature"];
    out = read<int8_t>(si.start, si.length);
}
void COMService::getBattery(uint8_t& out)
{
    const auto& si = Setting::Signal::handle()["battery_level"];
    out = read<uint8_t>(si.start, si.length);
}
void COMService::getLightSignals(bool& outl, bool& outr)
{
    const auto& si_l = Setting::Signal::handle()["left_light"];
    const auto& si_r = Setting::Signal::handle()["right_light"];
    outl = read<bool>(si_l.start, si_l.length);
    outr = read<bool>(si_r.start, si_r.length);
}
