#include <climits>
#include "setting.h"
#include "comservice.h"

/**
 * @brief Helper to insert a signal into the buffer, given a signal string.
 *        For internal use ONLY
 *
 * @param val Value to insert
 * @param sig_str Signal string
 */
void COMService::insert_helper(int val, const char *sig_str)
{
    // Get min and max values for signal
    const auto &signal_info = Setting::Signal::handle()[sig_str];

    // Lock mutex to insert into buffer
    std::lock_guard<std::mutex> lock(m_mtx);

    // Temporary 32-bit buffer
    uint32_t temp_buf{0};
    uint32_t buf_size = SBUFLEN * CHAR_BIT;

    // Fill up temporary buffer
    for (int i = (SBUFLEN - 1); i >= 0; i--)
    {
        temp_buf |= (m_buf[i] << (buf_size - ((SBUFLEN - i) * CHAR_BIT)));
    }

    // Bitmask to both clear and insert
    uint32_t mask = (1 << signal_info.length) - 1;

    // Clear existing bits at target location
    temp_buf &= ~(mask << signal_info.start);

    // Shift the new value into place
    temp_buf |= ((val & mask) << signal_info.start);

    // Write from temp_buf to m_buf
    for (int i = 0; i < SBUFLEN; i++)
    {
        m_buf[i] = (temp_buf >> (i * CHAR_BIT)) & 0xFF;
    }
}

/**
 * @brief Insert speed (Kph) into buffer
 *
 * @param speed_kph Speed in Kph (Kilometers per hour)
 */
void COMService::setSpeed(uint8_t speed_kph)
{
    insert_helper(speed_kph, "speed");
}

/**
 * @brief Insert temperature (°C) into buffer
 *
 * @param temp_c Temperature in Celsius
 */
void COMService::setTemp(int8_t temp_c)
{
    insert_helper(static_cast<int>(temp_c), "temperature");
}

/**
 * @brief Insert battery level (%) into buffer
 *
 * @param bat_prc Battery percentage
 */
void COMService::setBattery(uint8_t bat_prc)
{
    insert_helper(bat_prc, "battery_level");
}

/**
 * @brief Insert light signals into buffer
 *
 * @param left Left light
 * @param right Right light
 */
void COMService::setLightSignals(bool left, bool right)
{
    insert_helper(static_cast<int>(left), "left_light");
    insert_helper(static_cast<int>(right), "right_light");
}