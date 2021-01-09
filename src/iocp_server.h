#pragma once

#include <unordered_map>
#include <pthread.h>
#include <string.h>

namespace Network
{

class NetworkObject;
class IoHandler;

//对象池接口
class CNetworkObjPool
{
public:
	virtual NetworkObject* CreateAcceptedObject() = 0;
	virtual void DestroyAcceptedObject(NetworkObject* pNetworkObject) = 0;
	virtual void DestroyConnectedObject(NetworkObject* pNetworkObject) = 0;
};
//对象池接口模板
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
//消息解码
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
  uint32_t dwMaxAcceptSession;      // 最大接受连接数
  uint32_t dwMaxConnectSession;     // 最大主动连接数
  uint32_t dwMaxConnectBuffSize;    // 主动连接BuffSize
  uint32_t dwSendBufferSize;        // 发送缓存
  uint32_t dwRecvBufferSize;        // 接受缓存
  uint32_t dwTimeOut;               // 超时断开(秒)
  uint32_t dwMaxPacketSize;         // 最大包长
  CNetworkObjPool* pool;            // 对象池
  CMessageDecode * decode;          // 消息解码器
  bool openMsgQueue;                // 是否开启消息队列(防止服务器突发阻塞爆掉缓存)

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

	//白名单
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


