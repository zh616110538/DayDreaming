#include "Buffer.h"

std::shared_ptr<u_char> Buffer::read(size_t n, int mode)
{
	assert(n > 0);
	size_t data_size = size();
	if (mode == READWAIT && data_size < n)
	{
		return std::shared_ptr<u_char>();
	}
	size_t limit = n > data_size ? data_size : n;
	std::shared_ptr<u_char> p(new u_char[limit]);
	for (size_t i = 0; i < limit; ++i)
	{
		p.get()[i] = data.front();
		data.pop();
	}
	return p;
}