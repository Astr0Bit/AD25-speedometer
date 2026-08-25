#ifndef TCPCOM_H
#define TCPCOM_H

#include <QThread>
#include <unistd.h>
#include <sys/socket.h>
#include "comservice.h"

class TCPService : public QThread,
                   public COMService
{
    Q_OBJECT

private:
    int m_sockfd{-1};
    std::atomic_bool m_is_running{false};

    /**
     * @brief Creates the TCP/IP socket, spawns the main worker thread, and handles errors
     *
     */
    void run(void) override;

signals:
    // Notifies the Qt main thread if an error occured
    void fatalErrorOccurred(const QString &reason);

public:
    // Constructor to automatically start the server when created
    TCPService(QObject *parent = nullptr);

    // Destructor to clean up
    ~TCPService();
};

#endif