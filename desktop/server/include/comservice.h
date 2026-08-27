#ifndef COMSERVICE_H
#define COMSERVICE_H

#include <mutex>
#include <atomic>
#include <cstdint>
#include "setting.h"

// Abstract class for communication using multithreading
class COMService
{

public:
    // Destructor needed for inheritance
    virtual ~COMService() = default;

    // Methods for inserting:
    /**
     * @brief Insert speed (Kph) into buffer
     *
     * @param speed_kph Speed in Kph (Kilometers per hour)
     */
    void setSpeed(uint8_t speed_kph);

    /**
     * @brief Insert temperature (°C) into buffer
     *
     * @param temp_c Temperature in Celsius
     */
    void setTemp(int8_t temp_c);

    /**
     * @brief Insert battery level (%) into buffer
     *
     * @param bat_prc Battery percentage
     */
    void setBattery(uint8_t bat_prc);

    /**
     * @brief Insert light signals into buffer
     *
     * @param left Left light
     * @param right Right light
     */
    void setLightSignals(bool left, bool right);

    /**
     * @brief Get communication status
     *
     * @return true Connected
     * @return false Disconnected
     */
    bool getStatus() const
    {
        return m_status;
    };

protected:
    // Buffer, mutex, and atomic variables for class
    uint8_t m_buf[SBUFLEN]{0};
    mutable std::mutex m_mtx;
    std::atomic_bool m_status{false};

    /**
     * @brief Overwritten by the children, sends the buffer over the given protocol
     *
     */
    virtual void run(void) = 0;

private:
    /**
     * @brief Helper to insert a signal into the buffer, given a signal string.
     *        For internal use ONLY
     *
     * @param val Value to insert
     * @param sig_str Signal string
     */
    void insert_helper(int val, const char *sig_str);
};

#endif