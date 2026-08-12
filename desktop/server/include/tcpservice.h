#ifndef TCPCOM_H
#define TCPCOM_H

#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include "comservice.h"

class TCPService : public COMService
{
private:
    std::atomic_int m_sockfd{-1};
    std::atomic_bool m_is_running{true};

    // To later join the thread
    std::thread m_workerThread;

    /**
     * @brief Main thread for TCPService, reads the buffer, and periodically sends it over TCP / IP
     *
     */
    void serverWorker(int sockfd);

    /**
     * @brief Creates the TCP/IP socket, spawns the main worker thread, and handles errors
     *
     */
    void run(void) override;

public:
    // Constructor to automatically start the server when created
    TCPService()
    {
        run();
    }

    // Destructor to clean up
    ~TCPService()
    {
        m_is_running.store(false);

        // Close the socket
        if (m_sockfd != -1)
        {
            shutdown(m_sockfd, SHUT_RDWR);
            close(m_sockfd);
        }

        // Join the worker thread
        if (m_workerThread.joinable())
        {
            m_workerThread.join();
        }
    }
};

#endif