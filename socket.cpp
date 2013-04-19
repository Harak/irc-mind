#include	<stdbool.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<iostream>
#include	<string>
#include	<string.h>
#include	<unistd.h>
#include	"socket.hpp"

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
  
  if (!(proto = getprotobyname("TCP")))
    return false;
  if ((_fd = socket(AF_INET, SOCK_STREAM, proto->p_proto)) < 0)
    return false;

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
      ::recv(_fd, &tmp, 1, 0);
      buf += tmp;
    } while (tmp != '\n');
  
  return buf;
}

void		clientSocket::send(char const *buf)
{
  ::send(_fd, buf, strlen(buf), 0);
}

void		clientSocket::send(std::string const buf)
{
  ::send(_fd, buf.c_str(), strlen(buf.c_str()), 0);
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
