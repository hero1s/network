#include "Network.h"
#include "TLock.h"
#include "SessionPool.h"
#include "Session.h"
#include "SessionList.h"

#include <algorithm>

namespace Network {
SessionPool::SessionPool(uint32_t dwSize,
        uint32_t dwSendBufferSize,
        uint32_t dwRecvBufferSize,
        uint32_t dwMaxPacketSize,
        uint32_t dwTimeOutTick,
        uint32_t dwIndexStart,
        bool bAcceptSocket)
{
    m_pList = new SessionList;

    m_dwMaxSize        = dwSize;
    m_dwSendBufferSize = dwSendBufferSize;
    m_dwRecvBufferSize = dwRecvBufferSize;
    m_dwMaxPacketSize  = dwMaxPacketSize;
    m_dwTimeOutTick    = dwTimeOutTick;
    m_dwIndexStart     = dwIndexStart;
    m_bAcceptSocket    = bAcceptSocket;
    m_dwCurSize        = 0;

    Create();
}
SessionPool::~SessionPool()
{
    if (m_pList) delete m_pList;
}
void SessionPool::Create()
{
    Session* pSession;
    if (m_dwCurSize<m_dwMaxSize)
    {
        ++m_dwCurSize;
        pSession = new Session(m_dwSendBufferSize, m_dwRecvBufferSize, m_dwMaxPacketSize, m_dwTimeOutTick);

        pSession->SetIndex(m_dwIndexStart+m_dwCurSize);
        if (m_bAcceptSocket)
        {
            pSession->SetAcceptSocketFlag();
        }
        m_pList->push_back(pSession);
    }
}
Session* SessionPool::Alloc(CMessageDecode* pDecode,bool openMsgQueue)
{
    m_pList->Lock();
    if (m_pList->empty())
    {
        Create();
        if (m_pList->empty())
        {
            m_pList->Unlock();
            return NULL;
        }
    }

    Session* pSession = m_pList->front();
    if (NULL!=pSession)
    {
        pSession->Init();
        pSession->SetMsgDecode(pDecode);
        pSession->SetOpenMsgQueue(openMsgQueue);
    }
    m_pList->pop_front();

    m_pList->Unlock();

    return pSession;
}
void SessionPool::Free(Session* pSession)
{
    m_pList->Lock();

    assert(m_pList->size()<m_dwMaxSize);

    m_pList->push_back(pSession);

    m_pList->Unlock();
}
int SessionPool::GetMaxSize()
{
    return m_dwMaxSize;
}

}
	
