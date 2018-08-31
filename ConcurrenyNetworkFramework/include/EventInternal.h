#ifndef EVENTINTERNAL_H
#define EVENTINTERNAL_H

#include "Epoll.h"
#include "ClientBase.h"
#include <map>
#include <atomic>
#include <memory>

class EventInternal
{
    public:
        EventInternal();
        virtual ~EventInternal();
        void add(shared_ptr<ClientBase>);
        void del(int);
        void dispatch();
    protected:

    private:
        struct node
        {
            shared_ptr<ClientBase> cli;
            //std::atomic_flag isCalling;
        };
        std::map<int,node> climap;
        Epoll ev;
};

#endif // EVENTINTERNAL_H
