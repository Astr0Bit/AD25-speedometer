#include <climits>
#include "comservice.h"

// * Macros
#define BYTE 8

// * Helper function
// * NOTE: Works as intended
bool COMService::insert_helper(int val, const char *sig_str)
{
    if (!m_is_running)
    {
        return false;
    }

    // * Get min and max values for signal
    const auto &signal_info = m_signal[sig_str];

    // * Invalid range
    if ((val < signal_info.min) || (val > signal_info.max))
    {
        return false;
    }

    // * Lock mutex to insert into buffer
    std::lock_guard<std::mutex> lock(m_mtx);

    // * Temporary 32-bit buffer
    uint32_t temp_buf{0};
    uint32_t buf_size = SBUFLEN * BYTE;
    for (int i = SBUFLEN; i >= 0; i--)
    {
        temp_buf |= (m_buf[i] << (buf_size - ((SBUFLEN - i) * BYTE)));
    }

    // * Bitmask to both clear and insert
    uint32_t mask = (1 << signal_info.length) - 1;

    // * Clear existing bits at target location
    temp_buf &= ~(mask << signal_info.start);

    // * Shift the new value into place
    temp_buf |= ((val & mask) << signal_info.start);

    // * Write from temp_buf to m_buf
    for (int i = 0; i < SBUFLEN; i++)
    {
        m_buf[i] = (temp_buf >> (i * BYTE)) & 0xFF;
    }

    return true;
}

bool COMService::insertSpeed(uint8_t speed_kph)
{
    return insert_helper(speed_kph, "speed");
}

bool COMService::insertTemp(int8_t temp_c)
{
    return insert_helper(static_cast<int>(temp_c), "temperature");
}

bool COMService::insertBattery(uint8_t bat_prc)
{
    return insert_helper(bat_prc, "battery_level");
}

bool COMService::insertLightSignals(bool left, bool right)
{
    return (insert_helper(static_cast<int>(left), "left_light") &&
            insert_helper(static_cast<int>(right), "right_light"));
}