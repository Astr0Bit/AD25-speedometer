#ifndef TCPCOM_H
#define TCPCOM_H

#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include "comservice.h"

class TCPService : public COMService
{
private:
    int m_sockfd{-1};
    std::atomic_bool m_is_running{true};

    // To later join the thread
    std::thread m_worker;

    /**
     * @brief Creates the TCP/IP socket, spawns the main worker thread, and handles errors
     *
     */
    void run(void) override;

public:
    // Constructor to automatically start the server when created
    TCPService();

    // Helper method to get running status
    bool isRunning(void);

    // Destructor to clean up
    ~TCPService();
};

#endif