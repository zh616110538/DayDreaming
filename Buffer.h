#pragma once

#include <queue>
#include <memory>
#include <assert.h>

class Buffer
{
public:
	enum{READWAIT,READALL};
	explicit Buffer() {}
	explicit Buffer(const Buffer &other) = delete;
	Buffer& operator=(const Buffer &other) = delete;
	size_t size() { return data.size(); }
	std::shared_ptr<u_char> read(size_t n, int mode = READWAIT);
	void write(u_char *p, size_t n) { for (size_t i = 0; i < n; ++i)data.push(p[i]); }
	template <typename C>
	void write(const C &c) 
	{
		for (typename C::const_iterator it = c.begin(); it != c.end(); ++it)
			data.push(*it); 
	}
private:
	std::queue<u_char> data;
};

class RecvBuffer
{
public:
	RecvBuffer(Buffer& buf) :b(&buf){}
	size_t size() { return b->size(); }
	std::shared_ptr<u_char> read(size_t n, int mode = Buffer::READWAIT) { return b->read(n, mode); }
private:
	Buffer *b;
};

class SendBuffer
{
public:
	SendBuffer(Buffer& buf) :b(&buf) {}
	size_t size() { return b->size(); }
	void write(u_char *p, size_t n) { b->write(p, n); }
	template <typename C> void write(const C &c){b->write(c);}
private:
	Buffer *b;
};


//int ReadToBuffer(int fd, Buffer &b,size_t s)
//{
//	u_char buf[4096];
//	int ret = 0,total = 0;
//	while (true) {
//		ret = read(fd, buf, 4096);
//		total += ret;
//		if (ret <= 0)
//			break;
//		try {
//			b.write(buf, ret);
//		}
//		catch (std::exception &e)
//		{
//			throw e;
//		}
//	}
//	return total;
//}