
#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

namespace Network {

class Session;
class IoHandler;
class SessionPool;

typedef struct tagIOHANDLER_DESC IOHANDLER_DESC, * LPIOHANDLER_DESC;
class Acceptor {
    friend void* accept_epoll_event_thread(void* param);
    friend void* accept_thread(void* param);

public:
    Acceptor();
    ~Acceptor();

    void Init(IoHandler* pIoHandler);
    bool StartListen(const char* pIP, uint16_t wPort);
    void Shutdown();
    inline int IsListening() { return m_listenSocket!=INVALID_SOCKET; }
    inline SOCKET GetListenSocket() { return m_listenSocket; }
    inline uint16_t GetListeningPort() { return m_sockaddr.sin_port; }

private:
    IoHandler   * m_pIoHandler;
    SOCKET      m_listenSocket;
    SOCKADDR_IN m_sockaddr;
    int         m_bShutdown;
    pthread_t   m_hAcceptThread;

};

}

#endif // _ACCEPTOR_H_
	
	 


