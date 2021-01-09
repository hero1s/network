#pragma once

#define MAX_CONNECT_THREAD    16

namespace Network {

class IoHandler;
class Session;
class SessionList;

//=============================================================================================================================
/// 
/**
	@remarks
			Connect( char *, uint16_t )
	@par
	@par
			PreConnectedList ConnectFailList
	@par
			ConnectFailListOnConnect(FALSE).
	@see
			Session
*/
//=============================================================================================================================
class Connector {
    friend void* connect_thread(void* param);

public:
    Connector(void);
    ~Connector(void);

    void Init(IoHandler* pIoHandler);
    void Connect(Session* pSession);
    void Shutdown();

private:
    IoHandler  * m_pIoHandler;
    SessionList* m_pConnectingList;
    pthread_t  m_hThread;
    int        m_bShutdown;

    sem_t m_semConnect;
};

}
