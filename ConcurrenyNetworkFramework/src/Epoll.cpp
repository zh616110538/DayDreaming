#include "Epoll.h"
#include <Common.h>

Epoll::Epoll()
{
    //ctor
    events = new struct epoll_event[1024];
    epollfd = epoll_create1(0);
}

Epoll::~Epoll()
{
    //dtor
    delete [] events;
}

void Epoll::add(int fd)
{
    struct epoll_event ev = {0,{0}};
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        throw "epoll_ctl add  error";
    }
}

void Epoll::del(int fd)
{
    struct epoll_event ev = {0,{0}};
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if(epoll_ctl(epollfd,EPOLL_CTL_DEL,events[n].data.fd,&ev) == -1)
    {
        throw "epoll_ctl del error";
    }
}

void Epoll::mod(int fd)
{

}

vector<pair(int,int)> Epoll::dispatch()
{
    int nfds = -1;
    vector<pair(int,int)> v;
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    if (nfds == -1)
    {
        throw "epoll_wait error";
    }
    for (size_t n = 0; n < nfds; ++n)
    {
        if(events[n].events & EPOLLIN)
            v.push_back(pair(events[n].data.fd,0x01));
    }
    return v;
}
