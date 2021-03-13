
#include "network.h"
#include "tlock.h"
#include "IoHandler.h"
#include "acceptor.h"
#include "connector.h"
#include "session_list.h"
#include "session_pool.h"
#include "session.h"
#include "send_buffer.h"
#include "recv_buffer.h"
#include "iocp_server.h"
#include "network_object.h"

namespace Network
{
void* epoll_thread(void* param)
{
	IoHandler* pIoHandler = (IoHandler*) param;

	pIoHandler->m_epoll = epoll_create(SOCKET_HOLDER_SIZE);
	if (pIoHandler->m_epoll == -1)
	{
		printf("Could not create epoll fd (/dev/epoll).");
		pthread_exit(0);
	}

	struct epoll_event* events = new struct epoll_event[SOCKET_HOLDER_SIZE];

	while (!pIoHandler->m_bShutdown)
	{
		int      fd_count = epoll_wait(pIoHandler->m_epoll, events, SOCKET_HOLDER_SIZE, 5000);
		for (int i        = 0; i < fd_count; i++)
		{
			Session* pSession = (Session*) events[i].data.ptr;

			if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR)
			{
				pSession->Remove();
				continue;
			}

			if (events[i].events & EPOLLOUT)
			{
				pSession->OnSend();
				pIoHandler->ModEpollEvent(pSession, EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP);
				//continue;
			}

			if (events[i].events & EPOLLIN)
			{
				//__sync_fetch_and_add ( &pSession->m_nRecvRef, 1 );
				pIoHandler->AddIoEvent(&events[i]);
			}
		}
	}
	delete[] events;

	pthread_exit(0);

}

//=============================================================================================================================
/**
	@remarks
	@param	param
*/
//=============================================================================================================================
void* io_thread(void* param)
{
	IoHandler* pIoHandler = (IoHandler*) param;

	Session* pSession = NULL;

	struct epoll_event event;

	while (1)
	{
		pIoHandler->m_lockEvents.Lock();
//		while ( pIoHandler->m_pEvents->GetLength() == 0 && !pIoHandler->m_bShutdown )			
//		{
		pIoHandler->m_condEvents.Wait(&pIoHandler->m_lockEvents);
//		}

		if (pIoHandler->m_bShutdown)
		{
			pIoHandler->m_lockEvents.Unlock();
			pthread_exit(NULL);
		}

		while (pIoHandler->m_pEvents->Dequeue(&event, 1))
		{
			pSession = (Session*) event.data.ptr;

			if (event.events == 0x800) // send data
			{
				pSession->DoSend(pIoHandler);
			}
			else if (event.events & EPOLLIN) // recv data
			{
				pSession->DoRecv();
			}
		}

		pIoHandler->m_lockEvents.Unlock();
	}
}

IoHandler::IoHandler()
{
	m_pAcceptSessionPool   = NULL;
	m_pConnectSessionPool  = NULL;
	m_pAcceptor            = NULL;
	m_pConnector           = NULL;
	m_pActiveSessionList   = NULL;
	m_pAcceptedSessionList = NULL;
	m_pConnectSuccessList  = NULL;
	m_pConnectFailList     = NULL;
	m_pTempList            = NULL;
	m_numActiveSessions    = 0;
	m_bShutdown            = FALSE;
	m_pEvents              = NULL;
	m_hIoThread            = 0;

	m_pNetworkPool         = NULL;
}

IoHandler::~IoHandler()
{
	if (!m_bShutdown)
		Shutdown();

	if (m_pAcceptSessionPool) delete m_pAcceptSessionPool;
	if (m_pConnectSessionPool) delete m_pConnectSessionPool;
	if (m_pAcceptor) delete m_pAcceptor;
	if (m_pConnector) delete m_pConnector;
	if (m_pActiveSessionList) delete m_pActiveSessionList;
	if (m_pAcceptedSessionList) delete m_pAcceptedSessionList;
	if (m_pConnectSuccessList) delete m_pConnectSuccessList;
	if (m_pConnectFailList) delete m_pConnectFailList;
	if (m_pTempList) delete m_pTempList;
	if (m_pEvents) delete m_pEvents;

}

void IoHandler::Init(IOCPServer* pIOCPServer, IOHANDLER_DESC& lpDesc)
{
	m_pIOCPServer  = pIOCPServer;
	m_pNetworkPool = lpDesc.pool;
	m_dwKey = lpDesc.dwIoHandlerKey;

	m_pActiveSessionList   = new SessionList;
	m_pAcceptedSessionList = new SessionList;
	m_pConnectSuccessList  = new SessionList;
	m_pConnectFailList     = new SessionList;
	m_pTempList            = new SessionList;

	m_dwMaxAcceptSession = lpDesc.dwMaxAcceptSession;
	m_pAcceptSessionPool = new SessionPool(lpDesc.dwMaxAcceptSession + EXTRA_ACCEPTEX_NUM,
	                                       lpDesc.dwSendBufferSize,
	                                       lpDesc.dwRecvBufferSize,
	                                       lpDesc.dwMaxPacketSize,
	                                       lpDesc.dwTimeOut,
	                                       1,
	                                       TRUE);

	m_pConnectSessionPool = new SessionPool(lpDesc.dwMaxConnectSession,
	                                        lpDesc.dwMaxConnectBuffSize,
	                                        lpDesc.dwMaxConnectBuffSize,
	                                        lpDesc.dwMaxPacketSize,
	                                        lpDesc.dwTimeOut,
	                                        m_pAcceptSessionPool->GetMaxSize() + 1,
	                                        FALSE);

	m_dwMaxPacketSize = lpDesc.dwMaxPacketSize;

	m_pEvents = new CircuitQueue<struct epoll_event>;
	m_pEvents->Create(SOCKET_HOLDER_SIZE*2, SOCKET_HOLDER_SIZE);

	pthread_create(&m_hEpollThread, NULL, epoll_thread, (void*) this);
	pthread_create(&m_hIoThread, NULL, io_thread, (void*) this);

}

uint32_t IoHandler::Connect(NetworkObject* pNetworkObject, const char* pszIP, uint16_t wPort)
{
	if (m_pConnector == NULL)
	{
		m_pConnector = new Connector;
		m_pConnector->Init(this);
	}

	if (pNetworkObject->m_pSession != NULL) return 0;

	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = inet_addr(pszIP);
	addr.sin_port        = htons(wPort);

	Session* pSession = AllocConnectSession();
	assert(pSession != NULL && "Connect dwMaxConnectSession");

	pSession->CreateSocket();
	pSession->SetSockAddr(addr);

	assert(pNetworkObject != NULL);
	pSession->BindNetworkObject(pNetworkObject);

	m_pConnector->Connect(pSession);

	return pSession->GetIndex();
}

//=============================================================================================================================
/**
	@remarks
	@retval	int
	@param	pIP
	@param	wPort
*/
//=============================================================================================================================
bool IoHandler::StartListen(const char* pIP, uint16_t wPort)
{
	assert(m_dwMaxAcceptSession > 0);

	if (m_pAcceptor == NULL)
	{
		m_pAcceptor = new Acceptor;
		m_pAcceptor->Init(this);
	}
	if (IsListening()) return true;

	if (m_pAcceptor == NULL || !m_pAcceptor->StartListen(pIP, wPort))
	{
		printf("Listen socket creation failed.\n");
		return false;
	}

	return true;
}

bool IoHandler::IsListening()
{
	if (m_pAcceptor == NULL)return false;

	return m_pAcceptor->IsListening();
}

//=============================================================================================================================
/**
	@remarks
*/
//=============================================================================================================================

Session* IoHandler::AllocAcceptSession()
{
	return m_pAcceptSessionPool->Alloc();
}

Session* IoHandler::AllocConnectSession()
{
	return m_pConnectSessionPool->Alloc();
}

void IoHandler::FreeSession(Session* pSession)
{
	//printf("[FreeSession][%d]\n", (int)pSession->GetSocket());
	pSession->CloseSocket();
	pSession->Init();
	if (pSession->IsAcceptSocket())
	{
		m_pAcceptSessionPool->Free(pSession);
	}
	else
	{
		m_pConnectSessionPool->Free(pSession);
	}
}


//=============================================================================================================================
/**
	@remarks
	@par
			ConnectSuccessList OnConnect(TRUE) ActiveSessionList.
*/
//=============================================================================================================================
void IoHandler::ProcessConnectSuccessList()
{
	Session* pSession;
	SESSION_LIST activeList;

	//
	m_pConnectSuccessList->Lock();
	m_pTempList->splice(*m_pConnectSuccessList);
	m_pConnectSuccessList->Unlock();

	while (!m_pTempList->empty())
	{
		pSession = m_pTempList->pop_front();

		if (AddEpollEvent(pSession))
		{
			pSession->OnConnect(TRUE);
			activeList.push_back(pSession);
		}
		else
		{

			//
			FreeSession(pSession);

			pSession->OnConnect(FALSE);
		}
	}

	if (!activeList.empty())
	{
		m_numActiveSessions += (uint32_t) activeList.size();

		m_pActiveSessionList->Lock();
		m_pActiveSessionList->splice(activeList);
		m_pActiveSessionList->Unlock();
	}
}

//=============================================================================================================================
/**
	@remarks
	@par
			ConnectFailList OnConnect(FALSE)
*/
//=============================================================================================================================
void IoHandler::ProcessConnectFailList()
{
	Session* pSession;

	m_pConnectFailList->Lock();

	while (!m_pConnectFailList->empty())
	{
		pSession = m_pConnectFailList->pop_front();

		FreeSession(pSession);

		pSession->OnConnect(FALSE);
	}

	m_pConnectFailList->Unlock();

}

//=============================================================================================================================
/**
	@remarks
			TempSessionList ActiveSessionList OnAccpet()
*/
//=============================================================================================================================
void IoHandler::ProcessAcceptedSessionList()
{
	Session* pSession;

	m_pAcceptedSessionList->Lock();
	m_pTempList->splice(*m_pAcceptedSessionList);
	m_pAcceptedSessionList->Unlock();

	SESSION_LIST activeList;

	while (!m_pTempList->empty())
	{
		pSession = m_pTempList->pop_front();

		if (m_numActiveSessions >= m_dwMaxAcceptSession)
		{
			printf("connection full! no available accept socket!\n");
			FreeSession(pSession);
			continue;
		}

		if (!IsWhiteIP(pSession->GetIPNumber()))
		{
			//printf("connection is not the whitelist ip %s !\n", pSession->GetIP());
			FreeSession(pSession);
			continue;
		}

		if (!AddEpollEvent(pSession))
		{
			FreeSession(pSession);
			continue;
		}

		NetworkObject* pNetworkObject = (m_pNetworkPool != NULL) ? m_pNetworkPool->CreateAcceptedObject() : NULL;
		assert(pNetworkObject);

		pSession->BindNetworkObject(pNetworkObject);

		pSession->OnAccept();

		++m_numActiveSessions;
		activeList.push_back(pSession);
	}

	if (!activeList.empty())
	{
		m_pActiveSessionList->Lock();
		m_pActiveSessionList->splice(activeList);
		m_pActiveSessionList->Unlock();
	}
}

//=============================================================================================================================
/**
	@remarks
			accept connect
*/
//=============================================================================================================================
void IoHandler::ProcessActiveSessionList()
{
	SESSION_LIST_ITER it;
	Session* pSession;

	for (it = m_pActiveSessionList->begin(); it != m_pActiveSessionList->end(); ++it)
	{
		pSession = *it;

		if (pSession->ShouldBeRemoved())
			continue;

		if (pSession->HasDisconnectOrdered())
		{
			if (pSession->GetSendBuffer()->GetLength() == 0)
			{
				pSession->Remove();
			}
		}
		else
		{
			if (pSession->IsAcceptSocket() && pSession->IsOnIdle())
			{
				pSession->OnLogString("Idle Session,idleTick:%d--curTick:%d,diff:%d",
				                      pSession->GetIdleTick(), time(NULL), (time(NULL) - pSession->GetLastSyncTick()));
				pSession->Remove();
				continue;
			}

			if (!pSession->ProcessRecvdPacket())
			{
				pSession->Remove();
			}
		}
	}
}

//=============================================================================================================================
/**
	@remarks	
	Here FreeSession(pSession), then io_thread call pSession->OnRecv or pSession->OnSend (:-(
	 
*/
//=============================================================================================================================
void IoHandler::KickDeadSessions()
{
	SESSION_LIST_ITER it, it2;
	Session* pSession;
	m_pActiveSessionList->Lock();
	for (it = m_pActiveSessionList->begin(); it != m_pActiveSessionList->end();)
	{
		pSession = *it;

		if (pSession->ShouldBeRemoved())
		{
			it2 = it++;
			m_pActiveSessionList->remove(it2);
			m_pTempList->push_back(pSession);
		}
		else
			it++;
	}
	m_pActiveSessionList->Unlock();

	while (!m_pTempList->empty())
	{
		Session* pSession = m_pTempList->pop_front();

		--m_numActiveSessions;

		NetworkObject* pNetworkObject = pSession->GetNetworkObject();

		pSession->UnbindNetworkObject();

		DelEpollEvent(pSession);

		FreeSession(pSession);

		pNetworkObject->OnDisconnect();

		if (pSession->IsAcceptSocket())
		{
			if (m_pNetworkPool != NULL)
			{
				m_pNetworkPool->DestroyAcceptedObject(pNetworkObject);
			}
		}
		else
		{
			if (m_pNetworkPool != NULL)
			{
				m_pNetworkPool->DestroyConnectedObject(pNetworkObject);
			}
		}
	}

	//m_pTempList->clear();
}

void IoHandler::ProcessSend()
{
	SESSION_LIST_ITER it;
	Session* pSession;

	// ActiveSessionList
	m_pActiveSessionList->Lock();
	for (it = m_pActiveSessionList->begin(); it != m_pActiveSessionList->end(); ++it)
	{
		pSession = *it;

		if (pSession->ShouldBeRemoved()) continue;

		if (pSession->PreSend(this) == FALSE)
		{
			//printf("[REMOVE][%d] pSession->PreSend() == FALSE\n", (int)pSession->GetSocket());
			pSession->Remove();
		}
	}
	m_pActiveSessionList->Unlock();
}

//=============================================================================================================================
/**
	@remarks
*/
//=============================================================================================================================
void IoHandler::KickAllSessions()
{
	SESSION_LIST_ITER it;

	m_pActiveSessionList->Lock();
	for (it = m_pActiveSessionList->begin(); it != m_pActiveSessionList->end(); ++it)
	{
		(*it)->Remove();
	}
	m_pActiveSessionList->Unlock();
}

//=============================================================================================================================
/**
	@remarks
			Accept Connect ActiveSessionList
	@par
*/
//=============================================================================================================================
void IoHandler::Update()
{
	ProcessActiveSessionList();

	if (!m_pAcceptedSessionList->empty())
	{
		ProcessAcceptedSessionList();
	}

	if (!m_pConnectSuccessList->empty())
	{
		ProcessConnectSuccessList();
	}
	if (!m_pConnectFailList->empty())
	{
		ProcessConnectFailList();
	}
	KickDeadSessions();

}

void IoHandler::Shutdown()
{
	m_bShutdown = true;

	close(m_epoll);

	KickAllSessions();

	ProcessActiveSessionList();

	KickDeadSessions();

	if (m_pAcceptor)
		m_pAcceptor->Shutdown();

	if (m_pConnector)
		m_pConnector->Shutdown();

	pthread_cancel(m_hEpollThread);
	pthread_join(m_hEpollThread, NULL);

	// wake up io_thread to exit
	m_condEvents.Broadcast();

	pthread_cancel(m_hIoThread);
	pthread_join(m_hIoThread, NULL);

}

bool IoHandler::ModEpollEvent(Session* pSession, uint32_t nEvent)
{
	// Add epoll event based on socket activity.
	struct epoll_event ev;
	memset(&ev, 0, sizeof(epoll_event));

	// use edge-triggered instead of level-triggered because we're using nonblocking sockets
	ev.events   = nEvent | EPOLLET;
	//ev.data.fd = pSession->GetSocket();
	ev.data.ptr = pSession;

	if (epoll_ctl(m_epoll, EPOLL_CTL_MOD, pSession->GetSocket(), &ev) != 0)
	{
		pSession->OnLogString("Epoll could not add event to epoll set on fd %u", pSession->GetSocket());
		return false;
	}
	return true;
}

bool IoHandler::AddEpollEvent(Session* pSession)
{
	// Add epoll event based on socket activity.
	struct epoll_event ev;
	memset(&ev, 0, sizeof(epoll_event));

	// use edge-triggered instead of level-triggered because we're using nonblocking sockets
	ev.events   = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP;
	//ev.events |= EPOLLOUT ;
	//ev.data.fd = pSession->GetSocket();
	ev.data.ptr = pSession;

	if (epoll_ctl(m_epoll, EPOLL_CTL_ADD, pSession->GetSocket(), &ev) != 0)
	{
		pSession->OnLogString("Epoll could not add event to epoll set on fd %u", pSession->GetSocket());
		return false;
	}
	return true;
}
void IoHandler::DelEpollEvent(Session* pSession)
{
	// Remove from epoll list.
	struct epoll_event ev;
	memset(&ev, 0, sizeof(epoll_event));
	//ev.data.fd = pSession->GetSocket();
	ev.data.ptr = pSession;
	ev.events   = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLONESHOT;

	if (epoll_ctl(m_epoll, EPOLL_CTL_DEL, pSession->GetSocket(), &ev))
		pSession->OnLogString("Epoll could not remove fd %u from epoll set", pSession->GetSocket());

}

void IoHandler::AddIoEvent(struct epoll_event* pEvent)
{
	m_lockEvents.Lock();
	m_pEvents->Enqueue(pEvent, 1);
	m_lockEvents.Unlock();

	// wake up a io_thread
	m_condEvents.Signal();

}
//°×Ãûµ¥
void IoHandler::AddWhiteListIP(uint32_t ip)
{
	m_mpWhiteListIp[ip] = 1;
}
void IoHandler::ClearWhiteListIP()
{
	m_mpWhiteListIp.clear();
}
bool IoHandler::IsWhiteIP(uint32_t ip)
{
	if (m_mpWhiteListIp.empty())return true;

	return m_mpWhiteListIp.find(ip) != m_mpWhiteListIp.end();
}

}


 
