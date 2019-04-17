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
	size_t n = buf.size();
	send(getfd(), buf.read(n), n, 0);
	buf.free();
}

void ClientFd::dowrite()
{

}