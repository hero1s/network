#pragma once

#include "circuit_queue.h"

namespace Network {
//=============================================================================================================================
/// 
//	send  GQCS completion Completion(int).
//=============================================================================================================================
class SendBuffer {
public:
    SendBuffer() { m_pQueue = NULL; }
    virtual ~SendBuffer() { if (m_pQueue) delete m_pQueue; }

    inline void Create(int nBufferSize, uint32_t dwExtraBuffeSize)
    {
        if (m_pQueue) delete m_pQueue;
        m_pQueue = new CircuitQueue<uint8_t>;
        m_pQueue->Create(nBufferSize, dwExtraBuffeSize);
        m_bComplete = TRUE;
    }
    inline void Clear()
    {
        m_pQueue->Clear();
        m_bComplete = true;
    }

    inline void Completion(int nBytesSend)
    {
        m_pQueue->Dequeue(NULL, nBytesSend);
        m_bComplete = true;
    }

    //  GetSendParam m_bComplete = FALSE .
    inline bool GetSendParam(uint8_t** ppSendPtr, int& nLength)
    {
        if (!IsReadyToSend())
        {
            nLength = 0;
            return false;
        }
        *ppSendPtr = m_pQueue->GetReadPtr(0);
        nLength     = m_pQueue->GetReadableLen();
        m_bComplete = false;
        return true;
    }

    inline bool Write(uint8_t* pMsg, uint16_t wSize)
    {
        if (!m_pQueue->Enqueue(pMsg, wSize))
            return false;
        return true;
    }

    inline uint32_t GetLength() { return m_pQueue->GetLength(); }

    inline bool IsReadyToSend() { return (m_bComplete && m_pQueue->GetLength()>0) ? true : false; }

private:
    bool                 m_bComplete;
    CircuitQueue<uint8_t>* m_pQueue;
};

}



