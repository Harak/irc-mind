#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

class clientSocket 
{
  int		_fd;
  int		_port;
  std::string	_host;

public:
  clientSocket(char const *, int const);
  clientSocket(std::string const, int const);
  ~clientSocket(){};

  std::string	recv();
  void		send(char const *);
  void		send(std::string const);
  bool		connect();
  void		setPort(int const);
  void		setHost(char const *);
  void		setHost(std::string const);
  int		getFd() const;
  void		close();
};

#endif
