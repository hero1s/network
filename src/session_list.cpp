#include "network.h"
#include "session_list.h"
#include "session.h"

namespace Network {

SessionList::SessionList()
{
}

SessionList::~SessionList()
{
    Clear();
}

void SessionList::Clear()
{
    Session* pSession;
    Lock();
    while (!empty())
    {
        pSession = pop_front();
        if (pSession)
            delete pSession;
    }
    Unlock();
}

}
 
 