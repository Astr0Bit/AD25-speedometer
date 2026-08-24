#include "setting.h"
#include "tcpservice.h"

#include <QDebug>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

// Public
TCPService::TCPService()
{
    // Attempt to create the TCP / IP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sockfd == -1)
    {
        qCritical() << "Failed to create TCP/IP socket!";
        std::exit(EXIT_FAILURE);
    }
    else
    {
        qDebug() << "Created TCP/IP socket";
    }

    // Create an internet socket address
    sockaddr_in servaddr{};

    // Assign IP and PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(Setting::TCP::SERVER_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // To bypass socket cooldown
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        qCritical() << "setsockopt failed...";
        close(sockfd);
        std::exit(EXIT_FAILURE);
    }

    // Bind the socket address to the socket
    if (0 == bind(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)))
    {
        qDebug() << "Spawned TCP/IP server worker thread";

        // Spawn and hand the sockfd and thread to the TCPService object
        m_sockfd = sockfd;
        m_worker = std::thread(&TCPService::run, this);
        m_is_running.store(true);
    }
    else
    {
        qCritical() << "Failed to bind servaddr to the socket...";
        close(sockfd);
        std::exit(EXIT_FAILURE);
    }
}

bool TCPService::isRunning()
{
    return m_is_running.load();
}

TCPService::~TCPService()
{
    m_is_running.store(false);

    // Close the socket
    if (m_sockfd != -1)
    {
        shutdown(m_sockfd, SHUT_RDWR);
        close(m_sockfd);
    }

    // Join the worker thread
    if (m_worker.joinable())
    {
        m_worker.join();
    }
}

// Protected
void TCPService::run(void)
{
    if (0 == listen(m_sockfd, Setting::TCP::N_CONNS))
    {
        qDebug() << "Listening for incoming connections...";

        // Outer loop -> Constantly checks for new incomming connection requests
        while (true)
        {
            // Grab IP and length of incomming packet
            sockaddr_in cli{};
            socklen_t len = sizeof(cli);

            int connfd = accept(m_sockfd, (sockaddr *)&cli, &len);

            // Established connection
            if (connfd >= 0)
            {
                qDebug() << "Server accepted the client...";
                m_status.store(true);

                while (m_is_running)
                {
                    // * Periodically send the COMService buffer

                    // Wait x milliseconds before sending the packet
                    std::this_thread::sleep_for(std::chrono::milliseconds(Setting::INTERVAL));

                    ssize_t bytes_sent = 0;

                    // Temporary buffer
                    uint8_t temp_buf[SBUFLEN]{0};

                    // Scope to lock the mutex, which is automatically released when it goes out of scope
                    {
                        std::lock_guard<std::mutex> lock(m_mtx);
                        memcpy(temp_buf, m_buf, sizeof(uint8_t) * SBUFLEN);
                    }

                    // Send the buffer
                    bytes_sent = send(connfd, temp_buf, SBUFLEN, MSG_NOSIGNAL);

                    // In case the connection silently drops
                    if (bytes_sent <= 0)
                    {
                        qDebug() << "Connection lost";
                        m_status.store(false);
                        break; // Drops down to the shutdown and close section
                    }
                }

                // Close the connection
                shutdown(connfd, SHUT_RDWR);
                close(connfd);
            }
            else
            {
                if (!m_is_running)
                {
                    break;
                }

                qCritical() << "Failed to accept the connection...";
            }
        }
    }
    else
    {
        qCritical() << "Failed to listen to the port...";
    }
}