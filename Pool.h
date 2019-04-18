#pragma once

#include <list>
#include <thread>
#include <mutex>
#include <assert.h>
#include <iostream>
typedef unsigned char u_char;

class Pool
{
public:
	Pool(size_t n = 1024 * 1024 * 16, size_t c = 4096) :p(new u_char[n]), totalsize(n), chunk(c), allocsize(0)
	{
		size_t loop = n / chunk;
		if (loop < 1) throw "Parameter error";
		for (size_t i = 0; i < loop; ++i)
		{
			l.push_back(Node(p + i * c));
		}
		cur = l.begin();
	}
	~Pool() { delete p; }
	//Pool(Pool &&other) noexcept :
	//	p(other.p), totalsize(other.totalsize), chunk(other.chunk), allocsize(other.allocsize),
	//	m(std::move(other.m)),l(std::move(other.l)),cur(other.cur){}
	Pool(const Pool &other) = delete;
	Pool& operator=(const Pool &rhs) = delete;
	u_char* malloc(size_t n)
	{
		auto curtmp = cur;
		if (static_cast<size_t>((p + totalsize) - (curtmp->p + curtmp->end)) <= n)
		{
			while (curtmp != l.end() && curtmp->p != p)
			{
				++curtmp;
			}
		}
		size_t freespace = 0;
		for (auto it = curtmp; it != l.end(); ++it)
		{
			freespace += (chunk - it->end);
		}
		if (freespace <= n)
			resize();
		else
			cur = curtmp;
		return domalloc(n);
	}
	void free(size_t n)
	{
		if (allocsize < n)return;
		std::unique_lock<std::mutex> lk(m);
		size_t nbak = n;
		for (std::list<Node>::iterator it = l.begin(); it != l.end(); )
		{
			if (n <= (it->end - it->start))
			{
				{
					std::unique_lock<std::mutex> lk(*it->pm);
					it->start += n;
				}
				break;
			}
			else
			{
				n -= (it->end - it->start);
				{
					std::unique_lock<std::mutex> lk(*it->pm);
					it->start = it->end = 0;
					l.push_back(std::move(*it));
					it = l.erase(it);
				}
			}
			if (n == 0 && it == cur)
				break;
		}
		allocsize -= nbak;
	}
	size_t isconsequent(size_t n)//返回0代表n个字节连续，否则返回最大连续长度
	{
		if (allocsize == 0)throw "No data";
		auto it = l.begin();
		while (it->start == it->end)
		{
			++it;
		}
		size_t conseqcount = 0;
		while (1)
		{
			if (it != l.end())
				conseqcount += (it->end - it->start);
			else
				return conseqcount;
			if (conseqcount >= n)
				return 0;
			if (it->end != chunk)
				return conseqcount;
			++it;
		}
	}
	u_char* get()
	{
		if (allocsize == 0)throw "No data";
		auto it = l.begin();
		while (it->start == it->end)
		{
			it->start = it->end = 0;
			l.push_back(std::move(*it));
			it = l.erase(it);
		}
		return it->p + it->start;
	}
	u_char* get(size_t shift)
	{
		if (allocsize == 0 || shift >= allocsize)throw "No data";
		auto it = l.begin();
		while (it->start == it->end)
		{
			it->start = it->end = 0;
			l.push_back(std::move(*it));
			it = l.erase(it);
		}
		for (; it != l.end(); ++it)
		{
			if (it->p + it->start + shift < it->p + it->end)
				break;
			shift -= it->end - it->start;
		}
		return it->p + it->start + shift;
	}
	void shrink(size_t n)
	{
		if (n > allocsize)throw "shrink error";
		auto it = cur;
		allocsize -= n;
		while (n != 0)
		{
			size_t validbytes = it->end - it->start;
			if (validbytes >= n) {
				it->end -= n;
				n = 0;
			}
			else
			{
				n -= validbytes;
				it->end = it->start;
				--it;
			}
		}
		cur = it;
	}
	size_t size()
	{
		return allocsize;
	}
private:
	struct Node
	{
		Node(u_char *x) :p(x), start(0), end(0), pm(new std::mutex) {}
		Node(Node && other) noexcept :p(other.p), start(other.start), end(other.end), pm(std::move(other.pm)) {}
		Node(const Node &other) = delete;
		Node& operator=(const Node &rhs) = delete;
		u_char* p;
		size_t start;
		size_t end;
		std::unique_ptr<std::mutex> pm;
	};

	void resize()
	{
		std::cout << "resize()" << std::endl;
		std::unique_lock<std::mutex> lk(m);
		size_t newsize = totalsize * 2;
		u_char *tmp = new u_char[newsize];
		size_t loop = newsize / chunk;
		assert(loop > 1);
		std::list<Node> ltmp;
		for (size_t i = 0; i < loop; ++i)
		{
			ltmp.push_back(Node(tmp + i * chunk));
		}
		size_t pos = 0;
		for (std::list<Node>::iterator itl = l.begin(); itl != l.end(); ++itl)
		{
			for (size_t i = itl->start, j = 0; i < itl->end; ++i, ++j)
			{
				tmp[j + pos] = *(itl->p + i);
			}
			pos += itl->end - itl->start;
		}
		auto ittmp = ltmp.begin();
		for (size_t i = 0; i < allocsize / chunk; ++i,++ittmp)
		{
			ittmp->end = chunk;
		}
		size_t remainder = allocsize % chunk;
		if (remainder != 0)
			ittmp->end = remainder;
		cur = ittmp;
		totalsize = newsize;
		l = std::move(ltmp);
		delete[]p;
		p = tmp;
	}
	void chunkmalloc(std::list<Node>::iterator it, size_t n)
	{
		std::unique_lock<std::mutex> lk(*it->pm);
		it->end += n;
		assert(it->end <= chunk);
	}
	u_char* domalloc(size_t n)
	{
		size_t nbak = n;
		//auto ret = l.begin()->p + l.begin()->end;
		auto ret = cur->p + cur->end;
		assert(ret < p + totalsize);
		//首先需要确定要用几个chunk
		size_t div = n / chunk;
		size_t last = n % chunk;
		size_t extra;
		if (last < (chunk - cur->end))
			extra = 0;
		else
			extra = 1;
		size_t loop = div + extra;
		//size_t loop = n/chunk+1-(cur->end-cur->start)
		for (size_t i = 0; i < loop; ++i)
		{
			size_t count = chunk - cur->end;
			n -= count;
			chunkmalloc(cur, count);
			++cur;
			assert(cur != l.end());
		}
		chunkmalloc(cur, n);
		allocsize += nbak;
		return ret;
	}
	
	u_char *p;
	size_t totalsize;
	size_t chunk;
	size_t allocsize;
	std::mutex m;
	std::list<Node> l;
	std::list<Node>::iterator cur;
};

