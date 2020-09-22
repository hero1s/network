#include "Network.h"
#include "TLock.h"
#include "Connector.h"
#include "Session.h"
#include "SessionList.h"
#include "IoHandler.h"

namespace Network {
//=============================================================================================================================
/**
	@remarks
	@par
			connect ConnSuccessListConnFailureList.
*/
//=============================================================================================================================
void* connect_thread(void* param)
{
    Connector* pClass = (Connector*) param;
    Session  * pSession;

    while (1)
    {
        sem_wait(&pClass->m_semConnect);

        if (pClass->m_bShutdown)
        {
            pthread_exit(NULL);
        }

        while (!pClass->m_pConnectingList->empty())
        {
            if (pClass->m_bShutdown)
            {
                pthread_exit(NULL);
            }

            pClass->m_pConnectingList->Lock();
            pSession = pClass->m_pConnectingList->front();
            pClass->m_pConnectingList->pop_front();
            pClass->m_pConnectingList->Unlock();

            int err = connect(pSession->GetSocket(), (SOCKADDR*) (pSession->GetSockAddr()), sizeof(SOCKADDR_IN));

            if (err==SOCKET_ERROR)
            {
                pClass->m_pIoHandler->m_pConnectFailList->Lock();
                pClass->m_pIoHandler->m_pConnectFailList->push_back(pSession);
                pClass->m_pIoHandler->m_pConnectFailList->Unlock();
            }
            else
            {
                SocketOpt::Nonblocking(pSession->GetSocket());
                SocketOpt::DisableBuffering(pSession->GetSocket());

                pClass->m_pIoHandler->m_pConnectSuccessList->Lock();
                pClass->m_pIoHandler->m_pConnectSuccessList->push_back(pSession);
                pClass->m_pIoHandler->m_pConnectSuccessList->Unlock();
            }
        }

    }

}

Connector::Connector(void)
{
    m_pConnectingList = NULL;
    m_bShutdown       = FALSE;
    m_hThread         = 0;
}

Connector::~Connector(void)
{
    if (!m_bShutdown)
        Shutdown();

    if (m_pConnectingList)
        delete m_pConnectingList;
}

//=============================================================================================================================
/**
	@remarks
			connect ConnSuccessList
	@param	pServer
*/
//=============================================================================================================================
void Connector::Init(IoHandler* pIoHandler)
{
    m_pIoHandler = pIoHandler;

    if (m_pConnectingList) delete m_pConnectingList;
    m_pConnectingList = new SessionList;

    if (sem_init(&m_semConnect, 0, 0)!=0)
    {
        perror("Semaphore initialization failed");
    }

    pthread_create(&m_hThread, NULL, connect_thread, (void*) this);

}

//=============================================================================================================================
/**
	@remarks
			connect 
	@param	pSession			
	@param	szIP			
	@param	wPort
			
*/
//=============================================================================================================================
void Connector::Connect(Session* pSession)
{
    m_pConnectingList->Lock();
    m_pConnectingList->push_back(pSession);
    m_pConnectingList->Unlock();

    sem_post(&m_semConnect);

}


//=============================================================================================================================
/**
	@remarks
			connect .
*/
//=============================================================================================================================
void Connector::Shutdown()
{
    m_bShutdown = TRUE;

    // wake up connect_thread to exit
    sem_post(&m_semConnect);

    pthread_cancel(m_hThread);

    pthread_join(m_hThread, NULL);

    sem_destroy(&m_semConnect);
}

}

