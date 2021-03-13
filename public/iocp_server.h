#pragma once

#include <unordered_map>
#include <pthread.h>
#include <string.h>
#include <vector>

namespace Network
{

class NetworkObject;
class IoHandler;

//对象池接口
class CNetworkObjPool
{
public:
    //对象管理
	virtual NetworkObject* CreateAcceptedObject() = 0;
	virtual void DestroyAcceptedObject(NetworkObject* pNetworkObject) = 0;
	virtual void DestroyConnectedObject(NetworkObject* pNetworkObject) = 0;
};

//对象池接口模板
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
        dwMaxAcceptSession      = 10000;                                                // 最大接受连接数
        dwMaxConnectSession     = 100;                                                  // 最大主动连接数
        dwMaxConnectBuffSize    = 1024*1024;                                            // 主动连接BuffSize
        dwSendBufferSize        = 1024*1024;                                            // 发送缓存
        dwRecvBufferSize        = 1024*1024;                                            // 接受缓存
        dwTimeOut               = 40;                                                   // 超时断开(秒)
        dwMaxPacketSize         = 32*1024;                                              // 最大包长
        pool                    = new CNetworkObjPoolTemplete<TObj>;                    // 对象池
    }

  uint32_t dwIoHandlerKey;
  uint32_t dwMaxAcceptSession;      // 最大接受连接数
  uint32_t dwMaxConnectSession;     // 最大主动连接数
  uint32_t dwMaxConnectBuffSize;    // 主动连接BuffSize
  uint32_t dwSendBufferSize;        // 发送缓存
  uint32_t dwRecvBufferSize;        // 接受缓存
  uint32_t dwTimeOut;               // 超时断开(秒)
  uint32_t dwMaxPacketSize;         // 最大包长
  CNetworkObjPool* pool;            // 对象池
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

	//白名单
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


