#include "Fd.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <memory>

using namespace std;

static int
make_socket_non_blocking(int sfd)
{
	int flags, s;

	flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1)
	{
		perror("fcntl");
		return -1;
	}

	return 0;
}

void SocketFd::doread(Event &ev)
{
	while (1)
	{
		struct sockaddr in_addr;
		socklen_t in_len;
		int infd, s;
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

		in_len = sizeof in_addr;
		infd = accept(getfd(), &in_addr, &in_len);
		if (infd == -1)
		{
			if ((errno == EAGAIN) ||
				(errno == EWOULDBLOCK))
			{
				/* We have processed all incoming
				   connections. */
				break;
			}
			else
			{
				perror("accept");
				break;
			}
		}

		s = getnameinfo(&in_addr, in_len,
			hbuf, sizeof(hbuf),
			sbuf, sizeof(sbuf),
			NI_NUMERICHOST | NI_NUMERICSERV);
		if (s == 0)
		{
			printf("Accepted connection on descriptor %d "
				"(host=%s, port=%s)\n", infd, hbuf, sbuf);
		}

		/* Make the incoming socket non-blocking and add it to the
		   list of fds to monitor. */
		s = make_socket_non_blocking(infd);
		if (s == -1)
			abort();
		std::shared_ptr<BufferFd> f(new ClientFd(infd));
		ev.add(f);
	}
}

void ClientFd::doread(Event &ev)
{
	static size_t sss = 0;
	++sss;
	while (buf.size() >= 4)
	{
		auto p = buf.read(4);
		short protocol = *reinterpret_cast<short *>(p);
		short size = *reinterpret_cast<short *>(p + 2);
		assert(protocol == -1);
		assert(size == 12);
		if (buf.size() < 4 + size)
			break;
		buf.free();
		p = buf.read(size);
		int type = *reinterpret_cast<int *>(p);
		long data = *reinterpret_cast<long *>(p + 4);
		assert(type == 1);
		buf.free();
	}
	//while (buf.size() >= 4) {
	//	auto p = buf.read(4);
	//	short protocol = *reinterpret_cast<short *>(p);
	//	short size = *reinterpret_cast<short *>(p + 2);
	//	assert(protocol == 0);
	//	assert(size == 8);
	//	if (buf.size() < size + 4) {
	//		cout << "<<<<" << endl;
	//		break;
	//	}
	//	buf.free();
	//	p = buf.read(size);
	//	int type = *reinterpret_cast<int *>(p);
	//	assert(type == 1);
	//	int data = *reinterpret_cast<int *>(p + 4);
	//	//cout << "type = "<<type <<" , data = "<< data << endl;
	//	buf.free();
	//}
}

void ClientFd::dowrite()
{

}