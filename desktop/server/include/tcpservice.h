#ifndef TCPCOM_H
#define TCPCOM_H

#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include "comservice.h"

class TCPService : public COMService
{
};

#endif