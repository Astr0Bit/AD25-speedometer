#ifndef COMSERVICE_H
#define COMSERVICE_H

#include <mutex>
#include <atomic>
#include <cstdint>
#include "setting.h"

class COMService
{
protected:
    COMService();
    virtual ~COMService() = default;
public:

    void extractSpeed(uint8_t& out);
    void extractTemp(int8_t& out);
    void extractBattery(uint8_t& out);
    void extractLightSignals(bool& out_left, bool& out_right);

    virtual void receiveBuffer(void) = 0;
    virtual bool getStatus() const = 0;

protected:
    std::atomic_bool m_is_connected{false};

    class BufferGuard {
    private:
        friend class COMService;
        BufferGuard(uint8_t(&buf)[SBUFLEN], std::mutex& mtx);
        BufferGuard(BufferGuard&&) = default;
        BufferGuard& operator=(BufferGuard&&) = default;

        std::unique_lock<std::mutex> m_lock;
    public:
        uint8_t(&buffer)[SBUFLEN];
    };

    BufferGuard lockBuffer();

private:
    std::mutex m_mtx;
    uint8_t m_buffer[SBUFLEN]{};
    const Setting::Signal& m_signal;

    uint8_t extract_nbits(uint32_t n, uint32_t off);
};

#endif