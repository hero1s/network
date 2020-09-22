
#include "Network.h"
#include "TLock.h"
#include "NetworkObject.h"
#include "Session.h"

namespace Network
{
NetworkObject::NetworkObject()
{
	m_pSession = NULL;
	m_strIP    = "";
	m_ipNumber = 0;
	m_uid      = 0;
}

NetworkObject::~NetworkObject()
{
	m_pSession = NULL;
}

void NetworkObject::Disconnect(bool bGracefulDisconnect)
{
	if (m_pSession)
	{
		m_pSession->Disconnect(bGracefulDisconnect);
	}
}

bool NetworkObject::Send(uint8_t* pMsg, uint16_t wSize)
{
	if (NULL == m_pSession)
	{
		return false;
	}
	if (m_pSession->HasDisconnectOrdered())
	{
		return false;
	}
	if (m_pSession->ShouldBeRemoved())
	{
		return false;
	}
	return m_pSession->Send(pMsg, wSize);
}

void NetworkObject::Redirect(NetworkObject* pNetworkObject)
{
	assert(pNetworkObject != NULL && "NULL Redirect");
	assert(m_pSession != NULL);

	m_pSession->BindNetworkObject(pNetworkObject);
}
void NetworkObject::SetSession(Session* pSession)
{
	m_pSession = pSession;
	if (pSession != NULL)
	{
		m_strIP    = pSession->GetIP();
		m_ipNumber = pSession->GetIPNumber();
	}
}

std::string NetworkObject::GetIP()
{
	return m_strIP;
}
uint32_t NetworkObject::GetIPNumber()
{
	return m_ipNumber;
}
uint32_t NetworkObject::GetUID()
{
	return m_uid;
}
void NetworkObject::SetUID(uint32_t uid)
{
	m_uid = uid;
}
void NetworkObject::OnAccept(uint32_t dwNetworkIndex)
{

}
void NetworkObject::OnDisconnect()
{

}
int NetworkObject::OnRecv(uint8_t* pMsg, uint16_t wSize)
{
	return 0;
}
void NetworkObject::OnConnect(bool bSuccess, uint32_t dwNetworkIndex)
{

}
void NetworkObject::OnLogString(const char* pszLog)
{
	printf("%s \n", pszLog);
}


}

	
