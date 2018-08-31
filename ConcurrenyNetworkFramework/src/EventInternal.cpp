#include "EventInternal.h"
#include <utility>
EventInternal::EventInternal()
{
    //ctor
}

EventInternal::~EventInternal()
{
    //dtor
}

void EventInternal::add(shared_ptr<ClientBase>)
{

}

void EventInternal::del(int)
{

}

void EventInternal::dispatch()
{
    while(1)
    {
        vector<pair(int,int)> readyfds = ev.dispatch();
        for(auto &i:readyfds)
        {
            int fd = std::get<0>(i);
            shared_ptr<ClientBase> tmp = climap.at(fd)->cli;
            unsigned char buf[4096];
            int ret = recv(tmp->getfd(),buf,4096,0);
            if(ret > 0)
            {
                tmp->pushread(buf,ret);
                tmp->readCallBack();
            }
            else
            {
                tmp->errorCallBack();
                del(fd);
            }
        }
    }
}
