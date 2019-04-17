#pragma once

#include "PoolBuffer.h"
#include "Event.h"

struct NonBuffertag{};
struct Buffertag {};

class Event;

class Fd
{
public:
	Fd(int f = -1):fd(f){}
	//virtual ~Fd();
	int getfd() { return fd; }
private:
	int fd;
};

class NonBufferFd :public Fd
{
public:
	typedef NonBuffertag category;
	NonBufferFd(int f = -1):Fd(f){}
	virtual void doread(Event &ev) = 0;
};

class BufferFd :public Fd
{
public:
	typedef Buffertag category;
	BufferFd(int f = -1) :Fd(f) {}
	//u_char* read(size_t n) {  }
	//void write(u_char *p, size_t n) { buf.write(p, n); }
	virtual void doread(Event &ev) = 0;
	virtual void dowrite() = 0;
	PoolBuffer buf;
private:
};

class SocketFd :public NonBufferFd
{
public:
	SocketFd(int f = -1) :NonBufferFd(f) {}
	virtual void doread(Event &ev);
private:
	//virtual void dowrite(SendBuffer &sdbuf) {}
};

class ClientFd :public BufferFd
{
public:
	ClientFd(int f = -1) :BufferFd(f) {}
	virtual void doread(Event &ev);
	virtual void dowrite();
};