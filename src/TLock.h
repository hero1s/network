#ifndef _TLOCK_H_
#define _TLOCK_H_

#include "Network.h"

namespace Network {
class TLock {
    friend class TCond;
public:
    TLock();

    ~TLock();

    inline void Lock()
    {
        pthread_mutex_lock(&mutex_);
    }

    inline void Unlock()
    {
        pthread_mutex_unlock(&mutex_);
    }

    inline bool TryLock()
    {
        return (pthread_mutex_trylock(&mutex_)==0);
    }

protected:
    pthread_mutex_t mutex_;
};

class TCond {
public:
    inline TCond()
    {
        pthread_cond_init(&cond_, NULL);
    }
    inline ~TCond()
    {
        pthread_cond_destroy(&cond_);
    }

    inline void Signal()
    {
        pthread_cond_signal(&cond_);
    }
    inline void Broadcast()
    {
        pthread_cond_broadcast(&cond_);
    }
    inline void Wait(TLock* lock)
    {
        pthread_cond_wait(&cond_, &lock->mutex_);
    }
    inline bool Wait(TLock* lock, int seconds)
    {
        timespec tv;
        tv.tv_nsec = 0;
        tv.tv_sec  = seconds;
        if (pthread_cond_timedwait(&cond_, &lock->mutex_, &tv)==0)
            return true;
        else
            return false;
    }

private:
    pthread_cond_t cond_;

};

class TGuard {
private:
    TLock& mtx_;

public:
    TGuard(TLock& mtx)
            :mtx_(mtx)
    {
        mtx_.Lock();
    }

    ~TGuard()
    {
        mtx_.Unlock();
    }

private:
    //TGuard operator = (const TGuard &gd) {}
};

}

#endif
