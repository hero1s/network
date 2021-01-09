
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <string.h>

namespace Network
{
typedef int                SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr    SOCKADDR;

#ifndef TRUE
#define TRUE    1
#define FALSE    0
#endif

#define INVALID_SOCKET    -1
#define    SOCKET_ERROR    -1

}



