#include "setting.h"
#include "tcpservice.h"

#include <QDebug>
#include <netdb.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

class TCPService : public COMService
{
    private:
    void serverWorker(void) {
        
    }

    public:
    void run(void) override
    {
        // Attempt to create the TCP / IP socket
        int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sockfd == -1)
        {
            std::cout << "Failed to create TCP/IP socket, exiting...\n";
            std::exit(EXIT_FAILURE);
        }

        // Create an internet socket address
        sockaddr_in servaddr{0};

        // Assign IP and PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

        // Bind the socket address to the socket
        if (0 == bind(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)))
        {
            // TODO -> Likely to be put in a thread instead, ie. constantly listen
            // TODO -> What if the connection silently drops?
            if (0 == listen(sockfd, N_CONNS))
            {
                // Grab IP and length of incomming packet
                sockaddr_in cli{0};
                socklen_t len = sizeof(cli);

                int connfd = accept(sockfd, (sockaddr *)&cli, &len);

                // Established connection
                if (connfd >= 0)
                {
                    std::cout << "Server accepted the client...\n";
                    m_is_connected = true;

                    // TODO -> Periodically send the COMService buffer
                }
                else
                {
                    std::cout << "Failed to accept the connection...\n";
                }
            }
            else
            {
                std::cout << "Failed to listen to the port...\n";
            }
        }
        else
        {
            std::cout << "Failed to bind servaddr to the socket...\n";
        }

        // Close the socket
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
    }
}