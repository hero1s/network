
#ifndef SOCKET_OPS_H
#define SOCKET_OPS_H

#include <sys/types.h>
#include <sys/socket.h>

using namespace Network;

namespace SocketOpt {
// Create file descriptor for socket i/o operations.
SOCKET CreateTCPFileDescriptor();

// Disable blocking send/recv calls.
bool Nonblocking(SOCKET fd);

// Enable blocking send/recv calls.
bool Blocking(SOCKET fd);

// Disable nagle buffering algorithm
bool DisableBuffering(SOCKET fd);

// Enables nagle buffering algorithm
bool EnableBuffering(SOCKET fd);

// Set internal buffer size to socket.
bool SetRecvBufferSize(SOCKET fd, uint32_t size);

// Set internal buffer size to socket.
bool SetSendBufferSize(SOCKET fd, uint32_t size);

// Set timeout, in seconds
bool SetTimeout(SOCKET fd, uint32_t timeout);

// Closes socket completely.
void CloseSocket(SOCKET fd);

// Sets SO_REUSEADDR
void ReuseAddr(SOCKET fd);

// set nonblock
bool SetNonBlock(SOCKET fd);
bool SetNCloseWait(SOCKET fd);

void InitSocketOpt(SOCKET fd);

};

#endif


