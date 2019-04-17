#pragma once

#include <memory>
#include <assert.h>
#include "Pool.h"

class PoolBuffer
{
public:
	enum { READWAIT, READALL };
	explicit PoolBuffer():lastp(nullptr), lastn(0),data(4096*16*2,4096*16) {}
	explicit PoolBuffer(const PoolBuffer &other) = delete;
	PoolBuffer& operator=(const PoolBuffer &other) = delete;
	u_char* malloc(size_t n) { return data.malloc(n); }
	//void free(size_t n) { data.free(n); }
	//u_char* get() { return data.get(); }
	size_t size() { return data.size(); }
	u_char* const read(size_t n)
	{
		size_t lenth = data.isconsequent(n);
		lastn = n;
		if (lenth == 0)
		{
			return data.get();
		}
		else
		{
			lastp = new u_char[n];
			size_t i = 0;
			for (; i < lenth; ++i)
				lastp[i] = data.get()[i];
			data.free(lenth);
			size_t j = i;
			for (; i < n; ++i)
				lastp[i] = data.get()[i-j];
			return lastp;
		}
	}
	void allocreturn(size_t n){ data.shrink(n); }
	void free()
	{
		if (lastp != nullptr) {
			delete lastp;
			lastp = nullptr;
		}
		data.free(lastn);
		lastn = 0;
	}
private:
	u_char *lastp;
	size_t lastn;
	Pool data;
};

//class RecvBuffer
//{
//public:
//	RecvBuffer(Buffer& buf) :b(&buf) {}
//	size_t size() { return b->size(); }
//	std::shared_ptr<u_char> read(size_t n, int mode = Buffer::READWAIT) { return b->read(n, mode); }
//private:
//	Buffer *b;
//};
//
//class SendBuffer
//{
//public:
//	SendBuffer(Buffer& buf) :b(&buf) {}
//	size_t size() { return b->size(); }
//	void write(u_char *p, size_t n) { b->write(p, n); }
//	template <typename C> void write(const C &c) { b->write(c); }
//private:
//	Buffer *b;
//};


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