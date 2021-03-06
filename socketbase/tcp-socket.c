#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include "tcp-socket.h"
#include "inet-address.h"

int tcp_open(void)
{
	return socket(AF_INET, SOCK_STREAM, 0);
}
int tcp_bind(int socket, int ip, int port)
{
	struct sockaddr_in address;

	bzero(&address, sizeof(address));
	inet_set_family(&address, AF_INET);
	inet_set_ip(&address, ip);
	inet_set_port(&address, port);
	return bind(socket, (struct sockaddr*)&address, sizeof(address));
}
int tcp_listen(int socket, int backlog)
{
	return listen(socket, backlog);
}
int tcp_accept(int socket)
{
	struct sockaddr_in address;
	socklen_t socklen = sizeof(address);

	bzero(&address, sizeof(address));
	return accept(socket, (struct sockaddr*)&address, &socklen); 
}
int tcp_connect(int socket, int ip, int port)
{
	struct sockaddr_in address;

	bzero(&address, sizeof(address));
	inet_set_family(&address, AF_INET);
	inet_set_ip(&address, ip);
	inet_set_port(&address, port);
	return connect(socket, (struct sockaddr*)&address, sizeof(address));
}
int tcp_connects(int socket, const char *ip, int port)
{
	if( ip )
	{
		struct sockaddr_in address;

		inet_set_ipstring(&address, ip);
		return tcp_connect(socket, inet_get_ip(&address), port);
	}
	return -1;
}
int tcp_send(int socket, char *data, int len)
{
	if( data && len > 0 )
	{
		return send(socket, (const void*)data, (size_t)len, 0);
	}
	return -1;
}
int tcp_recv(int socket, char *buf, int len)
{
	if( buf && len > 0 )
	{
		return recv(socket, (void*)buf, (size_t)len, 0);
	}
	return -1;
}
int tcp_reuse_address(int socket)
{
	int value = 1;
	socklen_t len = sizeof(value);

	return setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (void*)&value, len);
}
int tcp_keepalive(int fd, int keep, int idle, int intvl, int cout)
{
	return
	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keep, sizeof(keep)) == 0
	&& setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&idle, sizeof(idle)) == 0
	&& setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void*)&intvl, sizeof(intvl)) == 0
	&& setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void*)&cout, sizeof(cout)) == 0;
}
int tcp_block(int socket)
{
	int value = 0;

	value = fcntl(socket, F_GETFL);
	value = value & ~(O_NONBLOCK);
	return fcntl(socket, F_SETFL, value);
}

int tcp_nonblock(int socket)
{
	int value = 0;

	value = fcntl(socket, F_GETFL);
	value = value | O_NONBLOCK;
	return fcntl(socket, F_SETFL, value);
}
int tcp_state(int socket)
{
	socklen_t len;
	struct tcp_info info;

	len = sizeof(info);
	bzero(&info, len);
	if( getsockopt(socket, IPPROTO_TCP, TCP_INFO, &info, &len) )
	{
		return -1;
	}
	return info.tcpi_state;
}
int tcp_close(int socket)
{
	return close(socket);
}
