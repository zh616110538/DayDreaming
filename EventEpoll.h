#pragma once

#include <memory>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

class ReadyFd
{
public:
	explicit ReadyFd(epoll_event &ev) :event(&ev) {}
	//explicit ReadyFd() {}
	//explicit ReadyFd(epoll_event *ev=nullptr):event(ev){}
	//ReadyFd& operator=(epoll_event *ev) { event = ev; return *this; }
	//explicit ReadyFd(ReadyFd& other) { event = other.event; }
	//ReadyFd& operator=(const ReadyFd& rhs) { event = rhs.event; return *this; }
	//explicit ReadyFd(ReadyFd&& rhs)noexcept :event(std::move(rhs.event)) {}
	bool isRead() const { return event->events & EPOLLIN; }
	bool isWrite() const { return event->events & EPOLLOUT; }
	bool isErr() const { return (event->events & EPOLLERR) || (event->events & EPOLLHUP); }
	int getfd() const { return event->data.fd; }
private:
	epoll_event *event;
};

class EventEpoll
{
public:
	enum{READ=EPOLLIN,WRITE=EPOLLOUT};
	EventEpoll(int n = 1024):maxfd(n),events(new struct epoll_event[n])
	{
		epollfd.reset(new Epollfd);
	}
	EventEpoll(const EventEpoll& other) 
	{ 
		epollfd = other.epollfd; 
		events = other.events;
	}
	EventEpoll& operator=(const EventEpoll& rhs)
	{
		return *this;
	}
	void add(int fd, u_int32_t t);
	void del(int fd);
	void mod(int fd, u_int32_t t);
	std::vector<ReadyFd> dispatch();
protected:

private:
	class Epollfd
	{
	public:
		Epollfd()
		{
			fd = epoll_create1(0);
		}
		~Epollfd()
		{
			close(fd);
		}
		int getfd()
		{
			return fd;
		}
	private:
		int fd;
	};
	int maxfd;
	std::shared_ptr<Epollfd> epollfd;
	std::shared_ptr<struct epoll_event> events;
};

