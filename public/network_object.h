
#pragma once

#include<string>

namespace Network
{

class Session;
//-------------------------------------------------------------------------------------------------
/// NetworkObject
//	- fnCreateAcceptedObject 
//	- (OnAccept, OnDisconnect, OnRecv, OnConnect)
//-------------------------------------------------------------------------------------------------
class NetworkObject
{
	friend class Session;
	friend class IoHandler;

public:
	NetworkObject();
	virtual ~NetworkObject();
	//true �����ر� false ������ر�
	void Disconnect(bool bGracefulDisconnect = true);
	bool Send(uint8_t* pMsg, uint16_t wSize);

	void Redirect(NetworkObject* pNetworkObject);
	std::string GetIP();
	uint32_t GetIPNumber();
	uint32_t GetUID();
	void SetUID(uint32_t uid);

protected:
	virtual void OnAccept(uint32_t dwNetworkIndex);
	virtual void OnDisconnect();
	virtual int OnRecv(uint8_t* pMsg, uint16_t wSize);
	virtual void OnConnect(bool bSuccess, uint32_t dwNetworkIndex);
	virtual void OnLogString(const char* pszLog);

	void SetSession(Session* pSession);

	Session* m_pSession;
	std::string m_strIP;
	uint32_t    m_ipNumber;
	uint32_t    m_uid;
};
}

	
	
