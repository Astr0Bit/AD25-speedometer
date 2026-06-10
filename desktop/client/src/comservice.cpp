#include <climits>
#include <cstring>
#include "comservice.h"

COMService::COMService()
    : m_signal{Setting::Signal::handle()}
{
}

COMService::BufferGuard::BufferGuard(uint8_t(&buf)[SBUFLEN], std::mutex& mtx)
    : buffer{buf}, m_lock{mtx}
{
}

COMService::BufferGuard COMService::lockBuffer()
{
    return COMService::BufferGuard{m_buffer, m_mtx};
}

inline uint8_t COMService::extract_nbits(uint32_t bit_len, uint32_t bit_start)
{
    auto locked = lockBuffer();
    const uint32_t byte_idx = bit_start >> 3;
    const uint32_t bit_off = bit_start & 7U;

    const uint16_t lo = (uint16_t)locked.buffer[byte_idx];
    const uint16_t hi = (bit_off + bit_len > 8U ? (uint16_t)locked.buffer[byte_idx + 1] << 8 : 0U);

    return (uint8_t)(((lo | hi) >> bit_off) & ((1U << bit_len) - 1U));
}

void COMService::extractSpeed(uint8_t& out)
{
    const auto& si = m_signal["speed"];
    out = extract_nbits(si.length, si.start);
}
void COMService::extractTemp(int8_t& out)
{
    const auto& si = m_signal["temperature"];
    const uint8_t raw = extract_nbits(si.length, si.start);
    const uint8_t sign_bit = static_cast<uint8_t>(1U << (si.length - 1));
    out = static_cast<int8_t>((raw ^ sign_bit) - sign_bit);
}
void COMService::extractBattery(uint8_t& out)
{
    const auto& si = m_signal["battery_level"];
    out = extract_nbits(si.length, si.start);
}
void COMService::extractLightSignals(bool& out_left, bool& out_right)
{
    const auto& si_l = m_signal["left_light"];
    const auto& si_r = m_signal["right_light"];
    out_left = !!extract_nbits(si_l.length, si_l.start);
    out_right = !!extract_nbits(si_r.length, si_r.start);
}