#ifndef EVENTINTERNAL_H
#define EVENTINTERNAL_H

#include "ClientBase.h"
#include <map>
#include <atomic>

class EventInternal
{
    public:
        EventInternal();
        virtual ~EventInternal();

    protected:

    private:
        struct node
        {
            ClientBase *cli;
            //std::atomic_flag isCalling;
        };
        std::map<int,node> v;
};

#endif // EVENTINTERNAL_H
