#include <climits>
#include "comservice.h"

// * Resources used:
// * -

bool COMService::insertSpeed(uint8_t speed_kph)
{
    // * If the service is not running
    if (!m_is_running)
    {
        return false;
    }

    // * Get min and max values for signal
    const Setting::Signal::Info &speed_info = Setting::Signal::handle()["speed"];

    // * Invalid range
    if ((speed_kph < speed_info.min) || (speed_kph > speed_info.max))
    {
        return false;
    }

    // * Lock mutex to insert into buffer
    std::lock_guard<std::mutex> lock(m_mtx);

    // * Insert speed into buffer
    m_buf[speed_info.start] = speed_kph;

    return true;
}