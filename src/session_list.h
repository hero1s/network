#ifndef _SESSIONLIST_H_
#define _SESSIONLIST_H_

#include "double_list_t.h"
#include "tlock.h"

namespace Network {

class Session;
class SessionList : public Double_List_T<Session> {
public:
    SessionList();
    ~SessionList();

    void Clear();

    inline void Lock() { m_cs.Lock(); }
    inline void Unlock() { m_cs.Unlock(); }

private:
    TLock m_cs;
};

typedef SessionList           SESSION_LIST;
typedef SessionList::iterator SESSION_LIST_ITER;

}

#endif
	





