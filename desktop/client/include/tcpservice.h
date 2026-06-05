#ifndef TCPCOM_H
#define TCPCOM_H

#include <thread>
#include <unistd.h>
#include "comservice.h"
#include <sys/socket.h>

class TCPService : public COMService
{
};

#endif