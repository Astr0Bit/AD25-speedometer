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
    // * Buffer mutex, and atomic variable for class
    uint8_t m_buf[SBUFLEN]{0};
    mutable std::mutex m_mtx;
    std::atomic_bool m_is_connected{false};

public:
    // * Methods for inserting:
    /**
     * @brief Insert speed (Kph) into buffer
     *
     * @param speed
     */
    void insertSpeed(uint8_t speed_kph);

    /**
     * @brief Insert temperature (°C) into buffer
     *
     * @param temp
     */
    void insertTemp(int8_t temp_c);

    /**
     * @brief Insert battery level (%) into buffer
     *
     * @param bat_prc
     */
    void insertBattery(uint8_t bat_prc);

    /**
     * @brief Insert light signals into buffer
     *
     * @param is_on
     */
    void insertLightSignals(bool left, bool right);

    // * Send buffer
    virtual void sendBuffer(void) = 0;

    // * Get communication status
    virtual bool getStatus() const = 0;
};

#endif