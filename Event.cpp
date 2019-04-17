#include "Event.h"

Event::Event()
{
}

void Event::add(std::shared_ptr<BufferFd> f)
{
	ev.add(f->getfd(), EventEpoll::READ);
	fds.insert(std::pair<int, std::shared_ptr<BufferFd>>(f->getfd(),f));
}

void Event::add(std::shared_ptr<NonBufferFd> f)
{
	ev.add(f->getfd(), EventEpoll::READ);
	nonfds.insert(std::pair<int, std::shared_ptr<NonBufferFd>>(f->getfd(), f));
}

void Event::del(int fd)
{
	close(fd);
	if (fds.find(fd) != fds.end())fds.erase(fds.find(fd));
	if (nonfds.find(fd) != nonfds.end())nonfds.erase(nonfds.find(fd));
	//nonfds.erase(nonfds.find(fd));
}

void Event::dispatch()
{
	while (1)
	{
		//int n, i;
		std::vector<ReadyFd> readyfds = ev.dispatch();
		for (auto it = readyfds.begin(); it != readyfds.end(); ++it)
		{
			if (it->isErr())
			{
				del(it->getfd());
				continue;
			}
			int done = 0;
			if (it->isRead())
			{
				if (nonfds.find(it->getfd()) != nonfds.end())
				{
					nonfds.at(it->getfd())->doread(*this);
				}
				else
				{
					while (1)
					{
						ssize_t count;
						const size_t readsize = 4096;
						u_char *buf = fds.at(it->getfd())->buf.malloc(readsize);
						count = read(it->getfd(), buf, readsize);
						if (count == -1)
						{
							/* If errno == EAGAIN, that means we have read all
							   data. So go back to the main loop. */
							if (errno != EAGAIN)
							{
								done = 1;
								perror("read");
							}
							else
								fds.at(it->getfd())->buf.allocreturn(readsize);
							break;
						}
						else if (count == 0)
						{
							done = 1;
							break;
						}
						fds.at(it->getfd())->buf.allocreturn(readsize - count);
						if (count < static_cast<ssize_t>(readsize))
							break;
					}
					if (done == 1) {
						del(it->getfd());
						continue;
					}
					else
						fds.at(it->getfd())->doread(*this);
						//;
				}
			}
			if (it->isWrite())
			{
				fds.at(it->getfd())->dowrite();
			}
		}
	}
}