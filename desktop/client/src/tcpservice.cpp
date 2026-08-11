#include "tcpservice.h"
#include "comservice.h"

#include <QDebug>
//#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <chrono>
#include <cstring>
#include <cerrno>

TCPService::TCPService() : COMService{}
{
    m_worker = std::thread{&TCPService::run, this};
}
TCPService::~TCPService()
{
    m_stop = true;
    if (m_sockfd >= 0)
    {
        ::shutdown(m_sockfd, SHUT_RDWR);
        ::close(m_sockfd);
    }
    m_worker.join();
}
void TCPService::run()
{
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(Setting::TCP::PORT);
    if (1 != inet_pton(AF_INET, Setting::TCP::IP, &server_addr.sin_addr))
    {
        qCritical() << "Failed to convert ip-address '" << Setting::TCP::IP << "' to binary network format";
        return;
    }

    uint8_t tmpbuf[SBUFLEN];
    while (!m_stop)
    {
        if (!m_status)
        {
            if (m_sockfd >= 0)
            {
                ::shutdown(m_sockfd, SHUT_RDWR);
                ::close(m_sockfd);
            }
            m_sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
            if (m_sockfd < 0)
            {
                qCritical() << "Failed to create the socket: " << std::strerror(errno);
                return;
            }
            qDebug() << "Trying to connect to server...  IP:" << Setting::TCP::IP << " PORT:" << Setting::TCP::PORT;
            if (0 != ::connect(m_sockfd, (sockaddr*)&server_addr, sizeof(server_addr)))
            {
                qWarning() << "  Failed to connect to server: " << std::strerror(errno);
                std::this_thread::sleep_for(std::chrono::milliseconds{1000});
                continue;
            }
            m_status = true;
            qDebug() << "Connected to server";
        }

        if (SBUFLEN != ::read(m_sockfd, tmpbuf, SBUFLEN))
        {
            m_status = false;
            qWarning() << "Failed to read from server: " << std::strerror(errno);
            continue;
        }

        std::lock_guard<std::mutex> lock{m_mtx};
        std::memcpy(m_buffer, tmpbuf, SBUFLEN);
    }
}