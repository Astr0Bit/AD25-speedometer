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

public:
    /**
     * @brief Construct a new TCPService object
     *
     * @param parent Optional parent
     */
    TCPService(QObject *parent = nullptr);

    // Destructor to clean up
    ~TCPService();

signals:
    /**
     * @brief Notifies the Qt main thread if an error occured
     *
     * @param reason The reason for the error
     */
    void fatalErrorOccurred(const QString &reason);

private:
    int m_sockfd{-1};
    std::atomic_bool m_is_running{false};

    /**
     * @brief Creates the TCP/IP socket, spawns the main worker thread, and handles errors
     *
     */
    void run(void) override;
};

#endif