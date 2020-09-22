
#include "Network.h"
#include "TLock.h"
#include "Acceptor.h"
#include "IoHandler.h"
#include "IOCPServer.h"
#include "Session.h"
#include "RecvBuffer.h"
#include "SessionList.h"
#include "SessionPool.h"

namespace Network {

void* accept_thread(void* param)
{
    Acceptor* pAcceptor = (Acceptor*) param;
    while (!pAcceptor->m_bShutdown)
    {
        struct sockaddr_in addr;
        int                len  = sizeof(struct sockaddr_in);
        SOCKET             sock = accept(pAcceptor->m_listenSocket, (sockaddr*) &addr, (socklen_t*) & len);
        if (sock==-1)
        {
            continue;
        }

        SocketOpt::Nonblocking(sock);
        SocketOpt::DisableBuffering(sock);

        Session* pSession = pAcceptor->m_pIoHandler->AllocAcceptSession();
        if (pSession==NULL)
        {
            continue;
        }

        pSession->SetSocket(sock);
        pSession->SetSockAddr(addr);


        pAcceptor->m_pIoHandler->m_pAcceptedSessionList->Lock();
        pAcceptor->m_pIoHandler->m_pAcceptedSessionList->push_back(pSession);
        pAcceptor->m_pIoHandler->m_pAcceptedSessionList->Unlock();

    }
    pthread_exit(0);
}

Acceptor::Acceptor()
{
    m_listenSocket  = INVALID_SOCKET;
    m_hAcceptThread = 0;
    m_bShutdown     = 0;
}

Acceptor::~Acceptor()
{
    Shutdown();

}

void Acceptor::Init(IoHandler* pIoHandler)
{
    m_pIoHandler = pIoHandler;
}

bool Acceptor::StartListen(const char* pIP, uint16_t wPort)
{
    if (m_listenSocket!=INVALID_SOCKET)
    {
        return false;
    }

    m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_listenSocket==INVALID_SOCKET)
    {
        printf("\ncreate socket fail");
        return false;
    }

    SocketOpt::ReuseAddr(m_listenSocket);
//	SocketOpt::Nonblocking(m_listenSocket);
//	SocketOpt::SetTimeout(m_listenSocket, 30);

    memset(&m_sockaddr, 0, sizeof(m_sockaddr));

    m_sockaddr.sin_family      = AF_INET;
    m_sockaddr.sin_addr.s_addr = (pIP==NULL || strlen(pIP)==0) ? htonl(INADDR_ANY) : inet_addr(pIP);
    m_sockaddr.sin_port        = htons(wPort);

    int err = bind(m_listenSocket, (SOCKADDR*) &m_sockaddr, sizeof(m_sockaddr));

    if (err==SOCKET_ERROR)
    {
        SocketOpt::CloseSocket(m_listenSocket);
        printf("\nbind fail %s:%d -- %s\n", pIP, wPort, strerror(errno));
        m_listenSocket = INVALID_SOCKET;
        return false;
    }

    err = listen(m_listenSocket, 5);//SOMAXCONN );

    if (err==SOCKET_ERROR)
    {
        SocketOpt::CloseSocket(m_listenSocket);
        printf("\n[Acceptor::CreateListenSocket] socket listen fail!");
        m_listenSocket = INVALID_SOCKET;
        return false;
    }

    pthread_create(&m_hAcceptThread, NULL, accept_thread, (void*) this);

    return true;
}


//=============================================================================================================================
/**
	@remarks
*/
//=============================================================================================================================
void Acceptor::Shutdown()
{
    if (m_listenSocket!=INVALID_SOCKET)
    {
        SocketOpt::CloseSocket(m_listenSocket);
        m_bShutdown = 1;
        pthread_cancel(m_hAcceptThread);
        pthread_join(m_hAcceptThread, NULL);
    }

    m_listenSocket  = INVALID_SOCKET;
    m_hAcceptThread = 0;
    m_bShutdown     = 0;

}

}



