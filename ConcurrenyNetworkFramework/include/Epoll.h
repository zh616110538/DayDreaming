#ifndef EPOLL_H
#define EPOLL_H

#include "Common.h"

class Epoll
{
    public:
        Epoll();
        virtual ~Epoll();
        void add(int fd);
        void del(int fd);
        void mod(int fd);
        vector<pair(int,int)> dispatch();
    protected:

    private:
        int epollfd;
        struct epoll_event *events;
};

#endif // EPOLL_H
