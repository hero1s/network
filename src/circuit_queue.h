#ifndef _CIRCUITQUEUE_H_
#define _CIRCUITQUEUE_H_

#include <stdio.h>
#include <assert.h>
#include "tlock.h"

namespace Network {
template<typename T>
class CircuitQueue {
public:
    CircuitQueue()
            :m_pData(NULL), m_nLength(0), m_nSize(0), m_nHead(0), m_nTail(0)
    {
    }
    virtual ~CircuitQueue()
    {
        if (m_pData) delete[] m_pData;
    }
    void Create(int nSize, int nExtraSize = 0)
    {
        m_cs.Lock();
        if (m_pData) delete[] m_pData;

        m_pData      = new T[nSize+nExtraSize];
        m_nSize      = nSize;
        m_nExtraSize = nExtraSize;
        m_cs.Unlock();
    }
    inline void Clear()
    {
        m_cs.Lock();

        m_nLength = 0;
        m_nHead   = 0;
        m_nTail   = 0;

        m_cs.Unlock();
    }

    inline int GetSpace()
    {
        int iRet;

        m_cs.Lock();
        iRet = m_nSize-m_nLength;
        m_cs.Unlock();

        return iRet;
    }

    inline int GetLength()
    {
        int iRet;

        m_cs.Lock();
        iRet = m_nLength;
        m_cs.Unlock();

        return iRet;
    }

    inline int GetBackDataCount()
    {
        int iRet;

        m_cs.Lock();
        iRet = m_nSize-m_nHead;
        m_cs.Unlock();

        return iRet;
    }

    inline T* GetReadPtr(uint16_t wantReadSize)
    {
        T* pRet;

        m_cs.Lock();
        pRet = m_pData+m_nHead;
        int nSplitFirstDataCount;
        if (m_nHead>m_nTail && wantReadSize>0)
        {
            nSplitFirstDataCount = m_nSize-m_nHead;
            if (wantReadSize>nSplitFirstDataCount && (wantReadSize-nSplitFirstDataCount)<m_nExtraSize)
            {
                memcpy(m_pData+m_nSize, m_pData, sizeof(T)*(wantReadSize-nSplitFirstDataCount));
            }
        }
        m_cs.Unlock();
        return pRet;
    }

    inline T* GetWritePtr()
    {
        T* pRet;

        m_cs.Lock();
        pRet = m_pData+m_nTail;
        m_cs.Unlock();

        return pRet;
    }

    inline int GetReadableLen()
    {
        int iRet;

        m_cs.Lock();
        if (m_nHead==m_nTail)
        {
            iRet = GetLength()>0 ? m_nSize-m_nHead : 0;
        }
        else if (m_nHead<m_nTail)
        {
            iRet = m_nTail-m_nHead;
        }
        else
        {
            iRet = m_nSize-m_nHead;
        }
        m_cs.Unlock();

        return iRet;
    }

    inline int GetWritableLen()
    {
        int iRet;

        m_cs.Lock();
        if (m_nHead==m_nTail)
        {
            iRet = GetLength()>0 ? 0 : m_nSize-m_nTail;
        }
        else if (m_nHead<m_nTail)
        {
            iRet = m_nSize-m_nTail;
        }
        else
        {
            iRet = m_nHead-m_nTail;
        }
        m_cs.Unlock();

        return iRet;
    }

    inline bool Enqueue(T* pSrc, int nSize)
    {
        m_cs.Lock();

        if (GetSpace()<nSize)
        {
            m_cs.Unlock();
            return false;
        }
        if (pSrc)
        {
            if (m_nHead<=m_nTail)
            {
                int nBackSpaceCount = m_nSize-m_nTail;

                if (nBackSpaceCount>=nSize)
                {
                    memcpy(m_pData+m_nTail, pSrc, sizeof(T)*nSize);
                }
                else
                {
                    memcpy(m_pData+m_nTail, pSrc, sizeof(T)*nBackSpaceCount);
                    memcpy(m_pData, pSrc+nBackSpaceCount, sizeof(T)*(nSize-nBackSpaceCount));
                }
            }
            else
            {
                memcpy(m_pData+m_nTail, pSrc, sizeof(T)*nSize);
            }
        }
        m_nTail += nSize;
        m_nTail %= m_nSize;
        m_nLength += nSize;

        m_cs.Unlock();

        return true;
    }

    inline bool Dequeue(T* pTar, int nSize)
    {
        m_cs.Lock();

        if (!Peek(pTar, nSize))
        {
            m_cs.Unlock();
            return false;
        }

        m_nHead += nSize;
        m_nHead %= m_nSize;
        m_nLength -= nSize;

        m_cs.Unlock();

        return true;
    }
    inline bool Peek(T* pTar, int nSize)
    {
        m_cs.Lock();

        if (m_nLength<nSize)
        {
            m_cs.Unlock();
            return false;
        }

        if (pTar!=NULL)
        {
            if (m_nHead<m_nTail)
            {
                memcpy(pTar, m_pData+m_nHead, sizeof(T)*nSize);
            }
            else
            {
                if (GetBackDataCount()>=nSize)
                {
                    memcpy(pTar, m_pData+m_nHead, sizeof(T)*nSize);
                }
                else
                {
                    memcpy(pTar, m_pData+m_nHead, sizeof(T)*GetBackDataCount());
                    memcpy(pTar+GetBackDataCount(), m_pData, sizeof(T)*(nSize-GetBackDataCount()));
                }
            }
        }

        m_cs.Unlock();

        return true;
    }

    inline void CopyHeadDataToExtraBuffer(int nSize)
    {
        assert(nSize<=m_nExtraSize);
        m_cs.Lock();
        memcpy(m_pData+m_nSize, m_pData, nSize);
        m_cs.Unlock();
    }

protected:
    TLock m_cs;
    T     * m_pData;
    int   m_nLength;
    int   m_nSize;
    int   m_nHead;
    int   m_nTail;
    int   m_nExtraSize;
};

}

#endif










