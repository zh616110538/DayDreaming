#include "EventEpoll.h"
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <exception>
#include <assert.h>

using namespace std;

void EventEpoll::add(int fd, u_int32_t t)
{
	struct epoll_event ev;
	ev.events = t | EPOLLET;
	ev.data.fd = fd;
	if (epoll_ctl(epollfd->getfd(), EPOLL_CTL_ADD, fd, &ev) == -1)
	{
		throw "epoll_ctl add  error";
	}
}

void EventEpoll::del(int fd)
{
	//struct epoll_event ev = { 0,{0} };
	//ev.events = EPOLLIN;
	//ev.data.fd = fd;
	//if (epoll_ctl(epollfd.get()->get(), EPOLL_CTL_DEL, fd, &ev) == -1)
	//{
	//	throw "epoll_ctl del error";
	//}
}

void EventEpoll::mod(int fd, u_int32_t t)
{
	struct epoll_event ev;
	ev.events = t | EPOLLET;
	ev.data.fd = fd;
	if (epoll_ctl(epollfd->getfd(), EPOLL_CTL_ADD, fd, &ev) == -1)
	{
		throw "epoll_ctl add  error";
	}
}

std::vector<ReadyFd> EventEpoll::dispatch()
{
	int nfds = -1;
	nfds = epoll_wait(epollfd->getfd(), events.get(), maxfd, -1);
	if (nfds == -1)
	{
		throw "epoll_wait error";
	}
	//如果加入了计时器，这里还需要判断nfds==0的情况
	std::vector<ReadyFd> v(events.get(), events.get()+nfds);
	return v;
}