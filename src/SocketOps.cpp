
#include "Network.h"
#include "SocketOpt.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>

namespace SocketOpt {
// Create file descriptor for socket i/o operations.
SOCKET CreateTCPFileDescriptor()
{
    // create a socket for use with overlapped i/o.
    return socket(AF_INET, SOCK_STREAM, 0);
}

// Disable blocking send/recv calls.
bool Nonblocking(SOCKET fd)
{
    uint32_t arg = 1;
    return (::ioctl(fd, FIONBIO, &arg)==0);
}

// Disable blocking send/recv calls.
bool Blocking(SOCKET fd)
{
    uint32_t arg = 0;
    return (ioctl(fd, FIONBIO, &arg)==0);
}

// Disable nagle buffering algorithm
bool DisableBuffering(SOCKET fd)
{
    uint32_t arg = 1;
    return (setsockopt(fd, 0x6, 0x1, (const char*) &arg, sizeof(arg))==0);
}

// Enable nagle buffering algorithm
bool EnableBuffering(SOCKET fd)
{
    uint32_t arg = 0;
    return (setsockopt(fd, 0x6, 0x1, (const char*) &arg, sizeof(arg))==0);
}

// Set internal buffer size to socket.
bool SetSendBufferSize(SOCKET fd, uint32_t size)
{
    return (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*) &size, sizeof(size))==0);
}

// Set internal buffer size to socket.
bool SetRecvBufferSize(SOCKET fd, uint32_t size)
{
    return (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*) &size, sizeof(size))==0);
}

// Set internal timeout.
bool SetTimeout(SOCKET fd, uint32_t timeout)
{
    struct timeval to;
    to.tv_sec  = timeout;
    to.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char*) &to, (socklen_t)sizeof(to)) != 0) return false;
    return (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &to, (socklen_t)
    sizeof(to)) == 0);
}

// Closes a socket fully.
void CloseSocket(SOCKET fd)
{
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

// Sets reuseaddr
void ReuseAddr(SOCKET fd)
{
    uint32_t option = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*) &option, 4);
}
// set nonblock
bool SetNonBlock(SOCKET fd)
{
    int iOpts = fcntl(fd, F_GETFL);
    if (iOpts<0)
    {
        return false;
    }
    iOpts = iOpts | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, iOpts)<0)
    {
        return false;
    }
    return true;
}
bool SetNCloseWait(SOCKET fd)
{
    linger stLinger;
    stLinger.l_onoff  = 1;
    stLinger.l_linger = 0;
    return setsockopt(fd, SOL_SOCKET, SO_LINGER, &stLinger, sizeof(linger))==0;
}
void InitSocketOpt(SOCKET fd)
{
    //SocketOpt::Nonblocking( sock ); toney test
    SocketOpt::DisableBuffering(fd);
    SocketOpt::SetNonBlock(fd);
    SocketOpt::SetNCloseWait(fd);
    SocketOpt::SetRecvBufferSize(fd, 1024*1024);
    SocketOpt::SetRecvBufferSize(fd, 1024*1024);
}
}