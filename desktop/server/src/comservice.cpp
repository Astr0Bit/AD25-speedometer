#include <climits>
#include "comservice.h"

// * Resources used:
// * -

// * Just for testing
#include <bitset>
#include <iostream>
void COMService::printBuffer() const
{
    std::lock_guard<std::mutex> lock(m_mtx);
    std::cout << "Buffer (Bits 23 -> 0): \n";
    // Print m_buf[2], then m_buf[1], then m_buf[0]
    std::cout << std::bitset<8>(m_buf[2]) << " "
              << std::bitset<8>(m_buf[1]) << " "
              << std::bitset<8>(m_buf[0]) << std::endl;
}

// * Helper function
// TODO -> Verify behavior
// TODO -> Make this more dynamic using the SBUFLEN macro
bool COMService::insert_helper(int val, const char *sig_str)
{
    if (!m_is_running)
    {
        return false;
    }

    // * Get min and max values for signal
    const auto &signal_info = Setting::Signal::handle()[sig_str];

    // * Invalid range
    if ((val < signal_info.min) || (val > signal_info.max))
    {
        return false;
    }

    // * Lock mutex to insert into buffer
    std::lock_guard<std::mutex> lock(m_mtx);

    // * Temporary 32-bit buffer
    // * NOTE: Most likely incorrect order
    uint32_t temp_buf = (m_buf[2] << 16) | (m_buf[1] << 8) | m_buf[0];

    // * Bitmask to both clear and insert
    uint32_t mask = (1 << signal_info.length) - 1;

    // * Clear existing bits at target location
    temp_buf &= ~(mask << signal_info.start);

    // * Shift the new value into place
    temp_buf |= ((val & mask) << signal_info.start);

    // * Write from temp_buf to m_buf
    m_buf[0] = temp_buf & 0xFF;
    m_buf[1] = (temp_buf >> 8) & 0xFF;
    m_buf[2] = (temp_buf >> 16) & 0xFF;

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