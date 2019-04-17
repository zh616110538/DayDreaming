#pragma once

#include "EventEpoll.h"
#include "Fd.h"
#include <memory>
#include <map>

class Fd;
class BufferFd;
class NonBufferFd;

class Event
{
public:
	Event();
	void add(std::shared_ptr<BufferFd> f);
	void add(std::shared_ptr<NonBufferFd> f);
	//void del(std::shared_ptr<Fd> f);
	void del(int fd);
	void dispatch();
private:
	EventEpoll ev;
	std::map<int, std::shared_ptr<BufferFd>> fds;
	std::map<int, std::shared_ptr<NonBufferFd>> nonfds;
};

