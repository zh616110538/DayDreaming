#ifndef EPOLL_H
#define EPOLL_H


class Epoll
{
    public:
        Epoll();
        virtual ~Epoll();
        void add(int fd);
        void del(int fd);
        void mod(int fd);
        void dispatch();
    protected:

    private:
        int epollfd;
};

#endif // EPOLL_H
