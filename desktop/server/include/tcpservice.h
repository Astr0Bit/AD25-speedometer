#ifndef TCPCOM_H
#define TCPCOM_H

#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include "comservice.h"

// TODO -> These should be put inside shared/setting.h instead
constexpr int N_CONNS{1};
constexpr int PORT{1337};
constexpr int SEND_INTERVAL_MS{20};

class TCPService : public COMService
{
private:
    /**
     * @brief Main thread for TCPService, reads the buffer, and periodically sends it over TCP / IP
     *
     */
    void serverWorker(int sockfd);

public:
    /**
     * @brief Creates the TCP/IP socket, spawns the main worker thread, and handles errors
     *
     */
    void run(void) override;
};

#endif