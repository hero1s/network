#pragma once

#include <unordered_map>
#include <pthread.h>
#include <string.h>
#include <vector>

namespace Network
{

class NetworkObject;
class IoHandler;

//����ؽӿ�
class CNetworkObjPool
{
public:
    //�������
	virtual NetworkObject* CreateAcceptedObject() = 0;
	virtual void DestroyAcceptedObject(NetworkObject* pNetworkObject) = 0;
	virtual void DestroyConnectedObject(NetworkObject* pNetworkObject) = 0;
};

//����ؽӿ�ģ��
template<typename TObject>
class CNetworkObjPoolTemplete : public CNetworkObjPool
{
public:
	virtual NetworkObject* CreateAcceptedObject()
	{
		return new TObject();
	}
	virtual void DestroyAcceptedObject(NetworkObject* pNetworkObject)
	{
		if (pNetworkObject != NULL)delete (pNetworkObject);
	};
	virtual void DestroyConnectedObject(NetworkObject* pNetworkObject)
	{

	};
};


//-------------------------------------------------------------------------------------------------
/// I/O 
//-------------------------------------------------------------------------------------------------
class IOHANDLER_DESC
{
public:
    IOHANDLER_DESC()
    {
        memset(this, 0, sizeof(IOHANDLER_DESC));
    }
    template<typename TObj>
    void Init(uint32_t key)
    {
        dwIoHandlerKey          = key;
        dwMaxAcceptSession      = 10000;                                                // ������������
        dwMaxConnectSession     = 100;                                                  // �������������
        dwMaxConnectBuffSize    = 1024*1024;                                            // ��������BuffSize
        dwSendBufferSize        = 1024*1024;                                            // ���ͻ���
        dwRecvBufferSize        = 1024*1024;                                            // ���ܻ���
        dwTimeOut               = 40;                                                   // ��ʱ�Ͽ�(��)
        dwMaxPacketSize         = 32*1024;                                              // ������
        pool                    = new CNetworkObjPoolTemplete<TObj>;                    // �����
    }

  uint32_t dwIoHandlerKey;
  uint32_t dwMaxAcceptSession;      // ������������
  uint32_t dwMaxConnectSession;     // �������������
  uint32_t dwMaxConnectBuffSize;    // ��������BuffSize
  uint32_t dwSendBufferSize;        // ���ͻ���
  uint32_t dwRecvBufferSize;        // ���ܻ���
  uint32_t dwTimeOut;               // ��ʱ�Ͽ�(��)
  uint32_t dwMaxPacketSize;         // ������
  CNetworkObjPool* pool;            // �����
};

class IOCPServer
{
	friend void* send_thread(void* param);

public:
	IOCPServer();
	virtual ~IOCPServer();

	bool Init(std::vector<IOHANDLER_DESC>& lpDesc);
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
	void CreateIoHandler(IOHANDLER_DESC& lpDesc);

	pthread_t                                m_hSendThread;
	int                                      m_bShutdown;
	std::unordered_map<uint32_t, IoHandler*> m_mapIoHandlers;
};

}


