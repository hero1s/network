#pragma once

#include <unordered_map>
#include <pthread.h>
#include <string.h>

namespace Network
{

class NetworkObject;
class IoHandler;

//����ؽӿ�
class CNetworkObjPool
{
public:
	virtual NetworkObject* CreateAcceptedObject() = 0;
	virtual void DestroyAcceptedObject(NetworkObject* pNetworkObject) = 0;
	virtual void DestroyConnectedObject(NetworkObject* pNetworkObject) = 0;
};
//����ؽӿ�ģ��
template<typename TYPE>
class CNetworkObjPoolTemplete : public CNetworkObjPool
{
public:
	virtual NetworkObject* CreateAcceptedObject()
	{
		return new TYPE();
	}
	virtual void DestroyAcceptedObject(NetworkObject* pNetworkObject)
	{
		if (pNetworkObject != NULL)delete (pNetworkObject);
	};
	virtual void DestroyConnectedObject(NetworkObject* pNetworkObject)
	{

	};
};
//��Ϣ����
class CMessageDecode
{
public:
	virtual uint32_t GetHeadLen() = 0;
	virtual uint32_t GetPacketLen(const uint8_t* pData, uint16_t wLen) = 0;
	virtual uint32_t MaxTickPacket()
	{
		return 100;
	};
};

//-------------------------------------------------------------------------------------------------
/// I/O 
//-------------------------------------------------------------------------------------------------
typedef struct tagIOHANDLER_DESC
{
  uint32_t dwIoHandlerKey;
  uint32_t dwMaxAcceptSession;      // ������������
  uint32_t dwMaxConnectSession;     // �������������
  uint32_t dwMaxConnectBuffSize;    // ��������BuffSize
  uint32_t dwSendBufferSize;        // ���ͻ���
  uint32_t dwRecvBufferSize;        // ���ܻ���
  uint32_t dwTimeOut;               // ��ʱ�Ͽ�(��)
  uint32_t dwMaxPacketSize;         // ������
  CNetworkObjPool* pool;            // �����
  CMessageDecode * decode;          // ��Ϣ������
  bool openMsgQueue;                // �Ƿ�����Ϣ����(��ֹ������ͻ��������������)

  tagIOHANDLER_DESC()
  {
	  memset(this, 0, sizeof(tagIOHANDLER_DESC));
  }
} IOHANDLER_DESC, * LPIOHANDLER_DESC;

class IOCPServer
{
	friend void* send_thread(void* param);

public:
	IOCPServer();
	virtual ~IOCPServer();

	bool Init(LPIOHANDLER_DESC lpDesc, uint32_t dwNumberofIoHandlers);
	bool StartListen(uint32_t dwIoHandlerKey, const char* pIP, uint16_t wPort);
	void Update();
	void Shutdown();
	uint32_t Connect(uint32_t dwIoHandlerKey, NetworkObject* pNetworkObject, const char* pszIP, uint16_t wPort);
	int IsListening(uint32_t dwIoHandlerKey);
	uint32_t GetNumberOfConnections(uint32_t dwIoHandlerKey);

	//������
	void AddWhiteListIP(uint32_t dwIoHandlerKey, uint32_t ip);
	void ClearWhiteListIP(uint32_t dwIoHandlerKey);
	bool IsWhiteIP(uint32_t dwIoHandlerKey, uint32_t ip);
private:
	void CreateIoHandler(LPIOHANDLER_DESC lpDesc);

	pthread_t                                m_hSendThread;
	int                                      m_bShutdown;
	std::unordered_map<uint32_t, IoHandler*> m_mapIoHandlers;
};

}


