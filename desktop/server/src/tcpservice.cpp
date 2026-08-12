#include "setting.h"
#include "tcpservice.h"

#include <QDebug>
#include <netdb.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

// Private
void TCPService::serverWorker(int sockfd)
{
    if (0 == listen(sockfd, Setting::TCP::N_CONNS))
    {
        std::cout << "Listening for incoming connections...\n";

        // Outer loop -> Constantly checks for new incomming connection requests
        while (true)
        {
            // Grab IP and length of incomming packet
            sockaddr_in cli{};
            socklen_t len = sizeof(cli);

            int connfd = accept(sockfd, (sockaddr *)&cli, &len);

            // Established connection
            if (connfd >= 0)
            {
                std::cout << "Server accepted the client...\n";
                m_status.store(true);

                while (true)
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
                        std::cout << "Connection lost\n";
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

                std::cout << "Failed to accept the connection...\n";
            }
        }
    }
    else
    {
        std::cout << "Failed to listen to the port...\n";
    }
}

// Protected
void TCPService::run(void)
{
    // Attempt to create the TCP / IP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sockfd == -1)
    {
        std::cout << "Failed to create TCP/IP socket, exiting...\n";
        std::exit(EXIT_FAILURE);
    }
    else
    {
        std::cout << "Created TCP/IP socket\n";
    }

    // Create an internet socket address
    sockaddr_in servaddr{};

    // Assign IP and PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(Setting::TCP::SERVER_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket address to the socket
    if (0 == bind(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)))
    {
        std::cout << "Spawned TCP/IP server worker thread\n";

        // Spawn and hand the sockfd and thread to the TCPService object
        m_sockfd = sockfd;
        m_workerThread = std::thread(&TCPService::serverWorker, this, sockfd);
    }
    else
    {
        std::cout << "Failed to bind servaddr to the socket...\n";
    }
}