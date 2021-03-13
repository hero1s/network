
#include "network.h"
#include "session.h"
#include "session_pool.h"
#include "send_buffer.h"
#include "recv_buffer.h"
#include "network_object.h"
#include "IoHandler.h"

using namespace std;

namespace Network
{
//=============================================================================================================================
/**
	@remarks
	@param	dwSendBufferSize
	@param	dwRecvBufferSize
	@param	dwTimeOut
*/
//=============================================================================================================================
Session::Session(uint32_t dwSendBufferSize, uint32_t dwRecvBufferSize, uint32_t dwMaxPacketSize, uint32_t dwTimeOut)
{
	m_pSendBuffer = new SendBuffer();
	m_pSendBuffer->Create(dwSendBufferSize, 0);// modify toney

	m_pRecvBuffer = new RecvBuffer();
	m_pRecvBuffer->Create(dwRecvBufferSize, dwMaxPacketSize);

	m_dwTimeOut     = dwTimeOut;
	m_socket        = INVALID_SOCKET;
	m_bAcceptSocket = FALSE;
	m_bCanSend      = true;

	m_pNetworkObject = NULL;
	m_wMaxPacketSize = dwMaxPacketSize;

	ResetTimeOut();
}

Session::~Session()
{
	CloseSocket();

	if (m_pSendBuffer) delete m_pSendBuffer;
	if (m_pRecvBuffer) delete m_pRecvBuffer;
}

//=============================================================================================================================
/**
	@remarks
	@param	socket
	@param	sockaddr
	@retval	int
*/
//=============================================================================================================================
void Session::Init()
{
	m_pSendBuffer->Clear();
	m_pRecvBuffer->Clear();

	ResetKillFlag();

	m_bDisconnectOrdered = false;
	m_bCanSend           = TRUE;
	ResetTimeOut();
    m_RecvQueueMessage.clear();
    m_SendQueueMessage.clear();
}
//=============================================================================================================================
/**
	@remarks
	@param	pMsg
	@param	wSize
	@retval	int
*/
//=============================================================================================================================
bool Session::Send(uint8_t* pMsg, uint16_t wSize)
{
    return AddSendMessage(pMsg,wSize);

/*	if (m_pSendBuffer->Write(pMsg, wSize) == FALSE)
	{
		OnLogString("m_pSendBuffer->Write fail. data length = %d, %d,ip:%s", m_pSendBuffer->GetLength(), wSize, GetIP());
		Remove();
		return false;
	}*/
	return true;
}

bool Session::AddSendMessage(uint8_t* pMsg, uint16_t wSize)
{
    if(m_SendQueueMessage.size() > 10000)
    {
        OnLogString("addSendMessage is more %d, %d,ip:%s", m_SendQueueMessage.size(), GetIP());
        Remove();
        return false;
    }
    shared_ptr<CMessage> message(new CMessage(pMsg, wSize));
    m_SendQueueMessage.push(message);
    return true;
}

bool Session::SwapSendMessage()
{
    while(!m_SendQueueMessage.empty() && m_pSendBuffer->GetLength() > m_wMaxPacketSize)
    {
        auto message = m_RecvQueueMessage.pop();
        if(m_pSendBuffer->Write(message->Data(),message->Length()) == FALSE)
        {
            OnLogString("m_pSendBuffer->Write fail. data length = %d, %d,ip:%s", m_pSendBuffer->GetLength(), message->Length(), GetIP());
            Remove();
            return false;
        }
    }
    return true;
}

bool Session::ProcessRecvdPacket()
{
    uint32_t msgNum = 0;
    while (!m_RecvQueueMessage.empty() && (msgNum++) < m_pNetworkObject->MaxTickPacket())
    {
        auto message = m_RecvQueueMessage.pop();
        int  iRet    = m_pNetworkObject->OnRecv(message->Data(), message->Length());
        if (iRet < 0)
        {
            OnLogString("process msg return < 0,disconnect");
            return false;
        }
        ResetTimeOut();
    }
    return true;
}

//解码消息到消息队列
bool Session::DecodeMsgToQueue()
{
    if(m_pNetworkObject == NULL)return true;

	uint8_t* pPacket;
	while (m_pRecvBuffer->GetRecvDataLen() >= m_pNetworkObject->GetHeadLen() && (m_RecvQueueMessage.size() < m_pNetworkObject->MaxTickPacket()))
	{
		pPacket = m_pRecvBuffer->GetFirstPacketPtr(m_pNetworkObject->GetHeadLen());
		uint32_t iPacketLen = m_pNetworkObject->GetPacketLen(pPacket, m_pNetworkObject->GetHeadLen());
		if (iPacketLen >= m_wMaxPacketSize)
		{
			OnLogString("max packet is big than:%d,ip:%s", iPacketLen, GetIP());
			return false;
		}
		pPacket = m_pRecvBuffer->GetFirstPacketPtr(iPacketLen);
		if (pPacket == NULL)
			return true;
		//放入消息队列
		shared_ptr<CMessage> message(new CMessage(pPacket, iPacketLen));
        m_RecvQueueMessage.push(message);
		//移除缓存
		m_pRecvBuffer->RemoveFirstPacket(iPacketLen);

		ResetTimeOut();
	}
	return true;
}
//=============================================================================================================================
/**
	@remarks
			send.
	@retval int
			- TRUE.
			- send FALSE.
*/
//=============================================================================================================================
int Session::OnSend()
{
	OnLogString("[Session::OnSend]");

	m_lockSend.Lock();
	m_bCanSend = TRUE;
	m_lockSend.Unlock();

	return m_bCanSend;
}

int Session::PreSend(IoHandler* pIoHandler)
{
    SwapSendMessage();
	if (!m_bRemove && m_bCanSend && m_pSendBuffer->IsReadyToSend())
	{
		// add to io_thread
		struct epoll_event event;
		event.events   = 0x800;
		event.data.ptr = this;

		pIoHandler->AddIoEvent(&event);
	}
	return TRUE;
}

int Session::DoSend(IoHandler* pIoHandler)
{
	TGuard gd(m_lockSend);

	if (m_bCanSend && m_bRemove == FALSE)
	{
		uint8_t* buf;
		int len;
		if (m_pSendBuffer->GetSendParam(&buf, len) == FALSE)
			return TRUE;

		assert (len > 0);

		int ret = send(m_socket, buf, len, 0);
		if (ret == SOCKET_ERROR)
		{
			if (errno == EAGAIN)
			{
				ret = 0;
			}
			else
			{
				OnLogString("[Session::DoSend] send error = %d .", errno);
				return FALSE;
			}
		}

		if (ret < len)
		{
			// now the send buffer is full, wait EPOLLOUT then send
			m_bCanSend = FALSE;
			uint32_t event = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLERR | EPOLLHUP;
			pIoHandler->ModEpollEvent(this, event);

			OnLogString("[Session::DoSend] send ret = %d/1024, len = %d/1024 EAGAIN.", ret, len);
		}

		m_pSendBuffer->Completion(ret);
	}

	return TRUE;
}

//=============================================================================================================================
/**
	@remarks
			IOCP recv.
	@retval int
			recv FALSE.
*/
//=============================================================================================================================
int Session::DoRecv()
{
	TGuard gd(m_lockRecv);

	char* buf;
	int ret = 0, len = 0;

	while (m_bRemove == FALSE)
	{
		m_pRecvBuffer->GetRecvParam((uint8_t**) &buf, len);
		if (len <= 0)
		{
			OnLogString("[Session::OnRecv] no more recv buffer.");
			Remove();
			return FALSE;
		}

		ret = recv(m_socket, buf, len, 0);
		if (ret == SOCKET_ERROR)
		{
			if (errno == EAGAIN)
			{
				//OnLogString("[Session::OnRecv] recv error = EAGAIN .");
				return TRUE;
			}
			else
			{
				OnLogString("[Session::OnRecv] recv error = %d .", errno);
				Remove();
				return FALSE;
			}
		}
		if (ret == 0) // peer closed
		{
			OnLogString("[Session::OnRecv] recv ret = 0.peer disconnect");
			Remove();

			return FALSE;
		}
		m_pRecvBuffer->Completion(ret);
		if (ret < len)
			break;
	}

	// 开启队列模式
    if (!DecodeMsgToQueue())
    {
        OnLogString("DecodeMsg Error:ip:%s,Remove",GetIP());
        Remove();
    }

	return TRUE;
}

SOCKET Session::CreateSocket()
{
	int nRet  = 0;
	int nZero = 0;

	SOCKET newSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (newSocket == INVALID_SOCKET)
	{
		return newSocket;
	}

	SocketOpt::Nonblocking(newSocket);
	SocketOpt::DisableBuffering(newSocket);
    SetSocket(newSocket);
	return newSocket;
}

void Session::BindNetworkObject(NetworkObject* pNetworkObject)
{
	m_pNetworkObject = pNetworkObject;
	pNetworkObject->SetSession(this);
}

void Session::UnbindNetworkObject()
{
	if (m_pNetworkObject == NULL)
	{
		return;
	}
	m_pNetworkObject->SetSession(NULL);

	m_pNetworkObject = NULL;
}

void Session::OnAccept()
{
	ResetKillFlag();

	ResetTimeOut();

	m_pNetworkObject->OnAccept(GetIndex());
}

void Session::OnConnect(bool bSuccess)
{
	Init();

	NetworkObject* pNetworkObject = m_pNetworkObject;

	if (!bSuccess)
	{
		UnbindNetworkObject();
	}

	pNetworkObject->OnConnect(bSuccess, GetIndex());
}

void Session::OnLogString(const char* pszLog, ...)
{
	if (!m_pNetworkObject) return;

	char    szBuffer[512] = "";
	va_list pArguments;

	va_start(pArguments, pszLog);
	vsprintf(szBuffer, pszLog, pArguments);
	va_end(pArguments);

	m_pNetworkObject->OnLogString(szBuffer);
}

void Session::Disconnect(bool bGracefulDisconnect)
{
	if (bGracefulDisconnect)
	{
		Remove();
	}
	else
	{
		m_bDisconnectOrdered = true;
	}
}

}


