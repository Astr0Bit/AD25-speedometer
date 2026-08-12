#ifndef COMSERVICE_H
#define COMSERVICE_H

#include <mutex>
#include <atomic>
#include <cstdint>
#include "setting.h"

class COMService
{
protected:
    std::mutex m_mtx;
    std::atomic_bool m_status{false};
    uint8_t m_buffer[SBUFLEN]{};

    COMService() = default;
    virtual ~COMService() = default;
    virtual void run() = 0;

public:
    void getSpeed(uint8_t& out);
    void getTemp(int8_t& out);
    void getBattery(uint8_t& out);
    void getLightSignals(bool& outl, bool& outr);
    inline bool getStatus() const noexcept { return m_status; }

private:
    uint64_t extract64(size_t byte_off);
    template <typename T> T read(size_t bit_off, size_t bit_len);
};

#endif