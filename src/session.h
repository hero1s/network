
#pragma once

#include "double_list_t.h"
#include "socket_ops.h"
#include "tlock.h"
#include "message.h"

namespace Network
{

class SendBuffer;
class RecvBuffer;
class SessionPool;
class NetworkObject;
class Session;
class IoHandler;
class CMessageDecode;

//=============================================================================================================================
/**
	@remarks
	@par
			- OnAccept:		accept
			- OnDisConnect:	
			- OnRecv:		
			- OnConnect:	connect
			- OnUpdate:		IO Update
	@note

*/
//=============================================================================================================================
class Session : public LinkD_T<Session>
{
	friend void* io_thread(void* param);
	friend class SessionPool;

public:
	/// dwTimeOut .
	Session(uint32_t dwSendBufferSize, uint32_t dwRecvBufferSize, uint32_t dwMaxPacketSize, uint32_t dwTimeOut);
	virtual ~Session();

	void SetMsgDecode(CMessageDecode* pDecode);

	void Init();
	bool Send(uint8_t* pMsg, uint16_t wSize);

	int OnSend();
	int DoSend(IoHandler* pIoHandler);
	int DoRecv();
	int PreSend(IoHandler* pIoHandler);
	SOCKET CreateSocket();
	bool ProcessRecvdPacket();

	//解码消息到消息队列
	bool DecodeMsgToQueue();

	void BindNetworkObject(NetworkObject* pNetworkObject);
	void UnbindNetworkObject();
	void OnAccept();
	void OnConnect(bool bSuccess);
	void OnLogString(const char* pszLog, ...);

	inline void SetSocket(SOCKET socket)
	{
		m_socket = socket;
	}
	inline void SetSockAddr(SOCKADDR_IN& sockaddr)
	{
		m_sockaddr = sockaddr;
	}
	inline void CloseSocket()
	{
		SocketOpt::CloseSocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
	inline NetworkObject* GetNetworkObject()
	{
		return m_pNetworkObject;
	}
	inline SendBuffer* GetSendBuffer()
	{
		return m_pSendBuffer;
	}                ///<
	inline RecvBuffer* GetRecvBuffer()
	{
		return m_pRecvBuffer;
	}                ///<
	inline SOCKET GetSocket()
	{
		return m_socket;
	}                    ///<
	inline SOCKADDR_IN* GetSockAddr()
	{
		return &m_sockaddr;
	}        ///<
	inline char* GetIP()
	{
		return inet_ntoa(m_sockaddr.sin_addr);
	}    ///<
	inline unsigned long GetIPNumber()
	{
		return m_sockaddr.sin_addr.s_addr;
	}

	inline uint32_t GetIdleTick() const
	{
		return m_dwTimeOut ? m_dwLastSyncTick + m_dwTimeOut : 0;
	}
	inline uint32_t GetLastSyncTick()
	{
		return m_dwLastSyncTick;
	}
	inline int IsOnIdle()
	{
		return m_dwTimeOut ? GetIdleTick() < time(NULL) : FALSE;
	}
	inline uint32_t GetIndex()
	{
		return m_dwIndex;
	}
	inline int IsAcceptSocket()
	{
		return m_bAcceptSocket;
	}
	inline void SetAcceptSocketFlag()
	{
		m_bAcceptSocket = TRUE;
	}
	void Remove()
	{
		__sync_fetch_and_or(&m_bRemove, 1); /*m_bRemove = 1;*/ }
	inline void ResetKillFlag()
	{
		__sync_fetch_and_and(&m_bRemove, 0); /*m_bRemove = 0;*/ }
	inline int ShouldBeRemoved()
	{
		return m_bRemove;
	}
	void Disconnect(bool bGracefulDisconnect);
	inline bool HasDisconnectOrdered()
	{
		return m_bDisconnectOrdered;
	}

private:
	void SetIndex(uint32_t index)
	{
		m_dwIndex = index;
	}
	inline void ResetTimeOut()
	{
		m_dwLastSyncTick = time(NULL);
	}

	NetworkObject* m_pNetworkObject;
	SendBuffer   * m_pSendBuffer;
	RecvBuffer   * m_pRecvBuffer;

	SOCKET      m_socket;
	SOCKADDR_IN m_sockaddr;

	uint32_t     m_dwLastSyncTick;
	volatile int m_bRemove;
	uint32_t     m_dwTimeOut;
	uint32_t     m_dwIndex;
	int          m_bAcceptSocket;
	bool         m_bDisconnectOrdered;

	TLock m_lockRecv;
	TLock m_lockSend;
	int   m_bCanSend;
	CMessageDecode* m_pMsgDecode;
	LockedQueue<std::shared_ptr<CMessage> > m_QueueMessage;//消息队列
	uint16_t                                m_wMaxPacketSize;

};

}

	
 