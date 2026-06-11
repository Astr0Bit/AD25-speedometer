#ifndef COMSERVICE_H
#define COMSERVICE_H

#include <mutex>
#include <atomic>
#include <cstdint>
#include "setting.h"

// * Abstract class for communication using multithreading
class COMService
{
protected:
    // * Constructor
    COMService();

    // * Buffer, mutex, and atomic variables for class
    uint8_t m_buf[SBUFLEN]{0};
    mutable std::mutex m_mtx;
    std::atomic_bool m_is_running{false};
    std::atomic_bool m_is_connected{false};

private:
    const Setting::Signal &m_signal;

    bool insert_helper(int val, const char *sig_str);

public:
    // * Just for testing
    void printBuffer() const;

    // * Destructor needed for inheritance
    virtual ~COMService() = default;

    // * Methods for inserting:
    /**
     * @brief Insert speed (Kph) into buffer
     *
     * @param speed_kph
     * @return true
     * @return false
     */
    bool insertSpeed(uint8_t speed_kph);

    /**
     * @brief Insert temperature (°C) into buffer
     *
     * @param temp_c
     * @return true
     * @return false
     */
    bool insertTemp(int8_t temp_c);

    /**
     * @brief Insert battery level (%) into buffer
     *
     * @param bat_prc
     * @return true
     * @return false
     */
    bool insertBattery(uint8_t bat_prc);

    /**
     * @brief Insert light signals into buffer
     *
     * @param left
     * @param right
     * @return true
     * @return false
     */
    bool insertLightSignals(bool left, bool right);

    // * Send buffer
    virtual bool sendBuffer(void) = 0;

    // * Get communication status
    virtual bool getStatus() const = 0;
};

#endif