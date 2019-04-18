//#include <cstdio>
//#include <iostream>
//#include <errno.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <sys/time.h>
//#include <unistd.h>
//#include <stdbool.h>
//#include <fcntl.h>
//#include <string>
//#include "Buffer.h"
//
//using namespace std;
//
//template<typename T>
//void TypeToByte(unsigned char *byte,const T &data)
//{
//	const unsigned char *p = reinterpret_cast<const unsigned char*>(&data);
//	for (int i = 0; i < sizeof(T); ++i)
//	{
//		byte[i] = p[i];
//	}
//}
//
//template<typename T>
//void ByteToType(T &data,const unsigned char *byte)
//{
//	data = *reinterpret_cast<const T*>(byte);
//}
//
//template<typename T>
//T ByteToType(const unsigned char *byte)
//{
//	return *reinterpret_cast<const T*>(byte);
//}
//
//class OneClass
//{
//public:
//	OneClass(bool b = false, int a = 0, double c = 0) :bset(b), text("Hello world"), data1(a), data2(c) {}
//	shared_ptr<unsigned char> ToByte()
//	{
//		int i = 0;
//		shared_ptr<unsigned char> p(new unsigned char[size()]);
//		p.get()[i++] = bset;
//		for (int j = 0; j < sizeof(text); ++j)
//		{
//			p.get()[i++] = text[j];
//		}
//		TypeToByte(p.get() + i, data1);
//		i += sizeof(data1);
//		TypeToByte(p.get() + i, data2);
//		i += sizeof(data2);
//		return p;
//	}
//	void ToClass(shared_ptr<unsigned char> p)
//	{
//		int i = 0;
//		bset = ByteToType<bool>(p.get()+i);
//		i += sizeof(bset);
//		for (int j = 0; j < sizeof(text); ++j)
//			text[j] = p.get()[i + j];
//		i += sizeof(text);
//		data1 = ByteToType<int>(p.get() + i);
//		i += sizeof(data1);
//		data2 = ByteToType<double>(p.get() + i);
//	}
//	void show()
//	{
//		cout << data2 << endl;
//	}
//	size_t size()
//	{
//		return sizeof(bset) + sizeof(text) + sizeof(data1) + sizeof(data2);
//	}
//private:
//	bool bset;
//	char text[64];
//	int data1;
//	double data2;
//};
//
//void read(RecvBuffer b)
//{
//	auto it = b.read(13, Buffer::READALL);
//	cout << it.get() << endl;
//}
//
//void write(SendBuffer b)
//{
//	b.write(string("Hello world"));
//}

//int main()
//{
//	OneClass a;
//	OneClass b(true, 1, 3.3);
//	Buffer buf;
//	buf.write(b.ToByte().get(),b.size());
//	a.ToClass(buf.read(a.size()));
//	a.show();
//	getchar();
//	return 0;
//}

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include "EventEpoll.h"

#define MAXEVENTS 64



static int
create_and_bind(char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
	hints.ai_flags = AI_PASSIVE;     /* All interfaces */

	s = getaddrinfo(NULL, port, &hints, &result);
	if (s != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0)
		{
			/* We managed to bind successfully! */
			break;
		}

		close(sfd);
	}

	if (rp == NULL)
	{
		fprintf(stderr, "Could not bind\n");
		return -1;
	}

	freeaddrinfo(result);

	return sfd;
}

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

int
readin(int argc, char *argv[])
{
	int sfd, s;
	int efd;
	struct epoll_event event;
	struct epoll_event *events;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	sfd = create_and_bind(argv[1]);
	if (sfd == -1)
		abort();

	int reuse = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		perror("setsockopt fail:");
		return -1;
	}


	s = make_socket_non_blocking(sfd);
	if (s == -1)
		abort();

	s = listen(sfd, SOMAXCONN);
	if (s == -1)
	{
		perror("listen");
		abort();
	}

	efd = epoll_create1(0);
	if (efd == -1)
	{
		perror("epoll_create");
		abort();
	}

	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
	if (s == -1)
	{
		perror("epoll_ctl");
		abort();
	}

	/* Buffer where events are returned */
	events = (epoll_event *)calloc(MAXEVENTS, sizeof event);

	/* The event loop */
	while (1)
	{
		int n, i;

		n = epoll_wait(efd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i++)
		{
			if ((events[i].events & EPOLLERR) ||
				(events[i].events & EPOLLHUP) ||
				(!(events[i].events & EPOLLIN)))
			{
				/* An error has occured on this fd, or the socket is not
				   ready for reading (why were we notified then?) */
				fprintf(stderr, "epoll error\n");
				close(events[i].data.fd);
				continue;
			}

			else if (sfd == events[i].data.fd)
			{
				/* We have a notification on the listening socket, which
				   means one or more incoming connections. */
				while (1)
				{
					struct sockaddr in_addr;
					socklen_t in_len;
					int infd;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

					in_len = sizeof in_addr;
					infd = accept(sfd, &in_addr, &in_len);
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
						hbuf, sizeof hbuf,
						sbuf, sizeof sbuf,
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

					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;
					s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
					if (s == -1)
					{
						perror("epoll_ctl");
						abort();
					}
				}
				continue;
			}
			else
			{
				/* We have data on the fd waiting to be read. Read and
				   display it. We must read whatever data is available
				   completely, as we are running in edge-triggered mode
				   and won't get a notification again for the same
				   data. */
				int done = 0;

				while (1)
				{
					ssize_t count;
					char buf[512];

					count = read(events[i].data.fd, buf, sizeof buf);
					if (count == -1)
					{
						/* If errno == EAGAIN, that means we have read all
						   data. So go back to the main loop. */
						if (errno != EAGAIN)
						{
							perror("read");
							done = 1;
						}
						break;
					}
					else if (count == 0)
					{
						/* End of file. The remote has closed the
						   connection. */
						done = 1;
						break;
					}

					/* Write the buffer to standard output */
					//s = write(1, buf, count);
					//printf("\n");
					if (s == -1)
					{
						perror("write");
						abort();
					}
				}

				if (done)
				{
					printf("Closed connection on descriptor %d\n",
						events[i].data.fd);

					/* Closing the descriptor will make epoll remove it
					   from the set of descriptors which are monitored. */
					close(events[i].data.fd);
				}
			}
		}
	}

	free(events);

	close(sfd);
	close(efd);
	return EXIT_SUCCESS;
}

int main1(int argc, char *argv[])
{
	int sfd, s;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	sfd = create_and_bind(argv[1]);
	if (sfd == -1)
		abort();

	int reuse = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		perror("setsockopt fail:");
		return -1;
	}

	s = make_socket_non_blocking(sfd);
	if (s == -1)
		abort();

	s = listen(sfd, SOMAXCONN);
	if (s == -1)
	{
		perror("listen");
		abort();
	}

	EventEpoll ev;

	ev.add(sfd, EventEpoll::READ);

	/* The event loop */
	while (1)
	{
		//auto set = ev.dispatch();
		//for (auto it = set->begin(); it != set->end(); ++it)
		auto set = ev.dispatch();
		for (auto it = set.begin(); it != set.end(); ++it)
		{
			if (it->isErr())
			{
				fprintf(stderr, "epoll error\n");
				close(it->getfd());
				continue;
			}
			if (it->getfd() == sfd)
			{
				while (1)
				{
					struct sockaddr in_addr;
					socklen_t in_len;
					int infd;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
					in_len = sizeof in_addr;
					infd = accept(sfd, &in_addr, &in_len);
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
						hbuf, sizeof hbuf,
						sbuf, sizeof sbuf,
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
					ev.add(infd, EventEpoll::READ|EventEpoll::WRITE);
				}
				continue;
			}
			else
			{
				int done = 0;
				if (it->isRead())
				{
					
					while (1)
					{
						ssize_t count;
						char buf[512];

						count = read(it->getfd(), buf, sizeof buf);
						if (count == -1)
						{
							/* If errno == EAGAIN, that means we have read all
							   data. So go back to the main loop. */
							if (errno != EAGAIN)
							{
								perror("read");
								done = 1;
							}
							break;
						}
						else if (count == 0)
						{
							/* End of file. The remote has closed the
							   connection. */
							done = 1;
							break;
						}

						/* Write the buffer to standard output */
					}

					//if (done)
					//{
					//	printf("Closed connection on descriptor %d\n",
					//		it->getfd());

					//	/* Closing the descriptor will make epoll remove it
					//	   from the set of descriptors which are monitored. */
					//	close(it->getfd());
					//}
				}
				if (done != 1 && it->isWrite())
				{
					while (1)
					{
						ssize_t count;
						char buf[512] = "Hello world!!!!!!!!!!!!!";

						count = write(it->getfd(), buf, sizeof buf);
						if (count == -1)
						{
							/* If errno == EAGAIN, that means we have read all
							   data. So go back to the main loop. */
							if (errno != EAGAIN)
							{
								perror("write");
								done = 1;
							}
							break;
						}
						else if (count == 0)
						{
							/* End of file. The remote has closed the
							   connection. */
							done = 1;
							break;
						}

						/* Write the buffer to standard output */
					}

					//if (done)
					//{
					//	printf("Closed connection on descriptor %d\n",
					//		it->getfd());

					//	/* Closing the descriptor will make epoll remove it
					//	   from the set of descriptors which are monitored. */
					//	close(it->getfd());
					//}
				}
			}
		}
	}

	close(sfd);
	return EXIT_SUCCESS;
}

#include "Event.h"
#include <memory>

int main(int argc,char *argv[])
{
	std::ios::sync_with_stdio(false);
	int sfd, s;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	char port[] = "5000";
	sfd = create_and_bind(port);
	if (sfd == -1)
		abort();

	int reuse = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		perror("setsockopt fail:");
		return -1;
	}

	s = make_socket_non_blocking(sfd);
	if (s == -1)
		abort();

	s = listen(sfd, SOMAXCONN);
	if (s == -1)
	{
		perror("listen");
		abort();
	}

	Event ev;
	ev.add(std::shared_ptr<SocketFd>(new SocketFd(sfd)));
	ev.dispatch();
	return 0;
}

//#include "Pool.h"
//
//int main(int argc,char *argv[])
//{
//	Pool p;
//	while (1) {
//		auto x = p.malloc(4096);
//		for (int i = 0; i < 4096; ++i)
//			x[i] = 88;
//	}
//}
