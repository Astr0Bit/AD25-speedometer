#include "tcpservice.h"
#include "comservice.h"

#include <QDebug>
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
    if (m_status)
    {
        ::shutdown(m_sockfd, SHUT_RDWR);
    }
    m_worker.join();
    if (m_sockfd >= 0)
    {
        ::close(m_sockfd);
    }
}
void TCPService::run()
{
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(Setting::TCP::SERVER_PORT);
    if (1 != inet_pton(AF_INET, Setting::TCP::SERVER_IP, &server_addr.sin_addr))
    {
        qCritical() << "Failed to convert ip-address '" << Setting::TCP::SERVER_IP << "' to binary network format";
        return;
    }

    uint8_t tmpbuf[SBUFLEN];
    while (!m_stop)
    {
        if (!m_status)
        {
            m_sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
            if (m_sockfd < 0)
            {
                qCritical() << "Failed to create the socket: " << std::strerror(errno);
                return;
            }

            qDebug() << "Trying to connect to server...  IP:" << Setting::TCP::SERVER_IP << " PORT:" << Setting::TCP::SERVER_PORT;
            if (0 != ::connect(m_sockfd, (sockaddr*)&server_addr, sizeof(server_addr)))
            {
                ::close(m_sockfd);
                m_sockfd = -1;
                qWarning() << "  Failed to connect to server: " << std::strerror(errno);
                std::this_thread::sleep_for(std::chrono::milliseconds{Setting::TCP::CLIENT_RETRY_CONNECT_INTERVAL});
                continue;
            }
            m_status = true;
            qDebug() << "Connected to server";
        }

        size_t nread{0};
        bool ok{true};
        while (nread < SBUFLEN && !m_stop)
        {
            ssize_t n = ::read(m_sockfd, tmpbuf + nread, SBUFLEN - nread);
            if (n > 0)
            {
                nread += static_cast<size_t>(n);
            }
            else if (n < 0 && errno == EINTR)
            {
                continue;
            }
            else
            {
                ok = false;
                if (n == 0)
                {
                    qDebug() << "Server closed the connection";
                }
                else
                {
                    qWarning() << "Failed to read from server: " << std::strerror(errno);
                }
                break;
            }
        }

        if (ok && nread == SBUFLEN)
        {
            std::lock_guard<std::mutex> lock{m_mtx};
            std::memcpy(m_buffer, tmpbuf, SBUFLEN);
        }
        else
        {
            m_status = false;
            ::shutdown(m_sockfd, SHUT_RDWR);
            ::close(m_sockfd);
            m_sockfd = -1;
        }
    }
}