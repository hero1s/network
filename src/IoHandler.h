#ifndef _IOHANDLER_H_
#define _IOHANDLER_H_

#include "tlock.h"
#include "circuit_queue.h"
#include "iocp_server.h"
#include <vector>
#include <unordered_map>

using namespace std;

#define EXTRA_ACCEPTEX_NUM        10
#define SOCKET_HOLDER_SIZE    1024

namespace Network
{

class Session;
class SessionPool;
class Acceptor;
class Connector;
class SessionList;
class IOCPServer;
class NetworkObject;
struct tagIOHANDLER_DESC;

typedef tagIOHANDLER_DESC IOHANDLER_DESC, * LPIOHANDLER_DESC;

class IoHandler
{
	friend void* epoll_thread(void* param);
	friend void* io_thread(void* param);
	friend void* send_thread(void* param);
	friend void* connect_thread(void* param);
	friend void* accept_thread(void* param);
	friend class Acceptor;

public:
	IoHandler();
	~IoHandler();

	void Init(IOCPServer* pIOCPServer, LPIOHANDLER_DESC lpDesc);
	bool StartListen(const char* pIP, uint16_t wPort);
	void Update();
	void Shutdown();
	bool IsRunning()
	{
		return !m_bShutdown;
	}
	uint32_t Connect(NetworkObject* pNetworkObject, const char* pszIP, uint16_t wPort);
	bool IsListening();
	inline uint32_t GetNumberOfConnections()
	{
		return m_numActiveSessions;
	}
	inline uint32_t GetKey()
	{
		return m_dwKey;
	}

	void AddIoEvent(struct epoll_event* pEvent);
	bool ModEpollEvent(Session* pSession, uint32_t nEvent);
	bool AddEpollEvent(Session* pSession);
	void DelEpollEvent(Session* pSession);

	//°×Ãûµ¥
	void AddWhiteListIP(uint32_t ip);
	void ClearWhiteListIP();
	bool IsWhiteIP(uint32_t ip);
private:
	Session* AllocAcceptSession();
	Session* AllocConnectSession();
	void FreeSession(Session* pSession);
	void ProcessConnectSuccessList();
	void ProcessConnectFailList();
	void ProcessAcceptedSessionList();
	void ProcessActiveSessionList();
	void KickDeadSessions();
	void ProcessSend();
	void KickAllSessions();

	IOCPServer * m_pIOCPServer;
	SessionPool* m_pAcceptSessionPool;
	SessionPool* m_pConnectSessionPool;
	Acceptor   * m_pAcceptor;
	Connector  * m_pConnector;
	SessionList* m_pActiveSessionList;
	SessionList* m_pAcceptedSessionList;
	SessionList* m_pConnectSuccessList;
	SessionList* m_pConnectFailList;
	SessionList* m_pTempList;
	bool m_bShutdown;

	uint32_t m_dwKey;
	int      m_epoll;                            // epoll fd

	pthread_t m_hIoThread;  // IO
	pthread_t m_hEpollThread;

	CircuitQueue<struct epoll_event>* m_pEvents;
	TLock m_lockEvents;
	TCond m_condEvents;

	uint32_t m_dwMaxPacketSize;
	uint32_t m_numActiveSessions;
	uint32_t m_dwMaxAcceptSession;

	CNetworkObjPool* m_pNetworkPool;
	CMessageDecode * m_pMsgDecode;

	unordered_map<uint32_t, uint32_t> m_mpWhiteListIp;
	bool                              m_openMsgQueue;
};

}

#endif
	
	 
	  