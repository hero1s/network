
#ifndef _RECVBUFFER_H_
#define _RECVBUFFER_H_

#include "CircuitQueue.h"

namespace Network {

//=============================================================================================================================
/// 
//		recv  GQCS completion Completion(int)
//=============================================================================================================================
class RecvBuffer {
public:
    RecvBuffer() { m_pQueue = NULL; }
    virtual ~RecvBuffer() { if (m_pQueue) delete m_pQueue; }

    inline void Create(int nBufferSize, uint32_t dwExtraBufferSize)
    {
        if (m_pQueue) delete m_pQueue;
        m_pQueue = new CircuitQueue<uint8_t>;
        m_pQueue->Create(nBufferSize, dwExtraBufferSize);
    }

    inline void Completion(int nBytesRecvd) { m_pQueue->Enqueue(NULL, nBytesRecvd); }

    inline void Clear() { m_pQueue->Clear(); }

    inline void GetRecvParam(uint8_t** ppRecvPtr, int& nLength)
    {
        *ppRecvPtr = m_pQueue->GetWritePtr();
        nLength = m_pQueue->GetWritableLen();
    }

    inline uint16_t GetRecvDataLen() { return m_pQueue->GetLength(); }

    inline uint8_t* GetFirstPacketPtr(uint16_t nPktSize)
    {
        if (m_pQueue->GetLength()<nPktSize)
            return NULL;

        /*if( m_pQueue->GetBackDataCount() < nPktSize )
        {
            m_pQueue->CopyHeadDataToExtraBuffer( nPktSize - m_pQueue->GetBackDataCount() );
        }*/
        return m_pQueue->GetReadPtr(nPktSize);
    }

    inline void RemoveFirstPacket(uint16_t wSize) { m_pQueue->Dequeue(NULL, wSize); }

private:
    CircuitQueue<uint8_t>* m_pQueue;
};

}

#endif



