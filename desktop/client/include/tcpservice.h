#ifndef TCPCOM_H
#define TCPCOM_H

#include <thread>
#include <unistd.h>
#include <string_view>
#include "comservice.h"
#include <sys/socket.h>

class TCPService : public COMService
{
    int m_sockfd{-1};
    std::atomic_bool m_stop{false};
    std::thread m_worker;

    void run() override;
public:
    TCPService();
    ~TCPService();
};

#endif