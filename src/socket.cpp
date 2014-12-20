#include	<stdbool.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<iostream>
#include	<string>
#include	<string.h>
#include	<unistd.h>
#include	"socket.hpp"
#include	"socketexception.hpp"

clientSocket::clientSocket(std::string const host, int const port)
{
  _port = port;
  _host = host;
}

clientSocket::clientSocket(char const *host, int const port)
{
  _port = port;
  _host = std::string(host);
}

bool clientSocket::connect()
{
  struct protoent	*proto;
  struct sockaddr_in	sin;
  struct hostent	*hostinfo = NULL;
  int			optval = 1;
  
  if (!(proto = getprotobyname("TCP")))
    return false;
  if ((_fd = socket(AF_INET, SOCK_STREAM, proto->p_proto)) < 0)
    return false;

  setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

#ifdef __linux__
  setsockopt(_fd, SOL_SOCKET, MSG_NOSIGNAL, &optval, sizeof(optval));
#else
  setsockopt(_fd, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));
#endif

  if (!(hostinfo = gethostbyname(_host.c_str())))
    return false;
  
  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_addr = *(struct in_addr*)hostinfo->h_addr;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(_port);

  if (::connect(_fd, (struct sockaddr *)&sin, sizeof(struct sockaddr)))
      return false;

  return true;
}

std::string	clientSocket::recv()
{
  char		tmp;
  std::string	buf("");

  do
    {
      if (::recv(_fd, &tmp, 1, 0) < 0)
	throw new SocketException("Unable to read data");

      buf += tmp;
    } while (tmp != '\n');
  
  return buf;
}

void		clientSocket::send(char const *buf)
{
  if (::send(_fd, buf, strlen(buf), 0) < 0)
    throw new SocketException("Could not send data.");
}

void		clientSocket::send(std::string const buf)
{
  this->send(buf.c_str());
}

void		clientSocket::setPort(int const port)
{
  _port = port;
}

void		clientSocket::setHost(char const *host)
{
  setHost(std::string(host));
}

void		clientSocket::setHost(std::string const host)
{
  _host = host;
}

int		clientSocket::getFd() const
{
  return _fd;
}

void		clientSocket::close()
{
  ::close(_fd);
}
