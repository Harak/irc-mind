#ifndef __USER_HPP__
#define __USER_HPP__

#include <string>

class User
{
  std::string _nick;
  std::string _ident;
  std::string _host;
 
  std::string _ip;
  int	      _level;

public:
  User(){}
  User(std::string nick, std::string ident, std::string host) :
    _nick(nick), _ident(ident), _host(host) {}
  ~User(){}

  void setNick(std::string const);
  void setIdent(std::string const);
  void setHost(std::string const);
  void setIp(std::string const);
  void setLevel(int const);

  std::string getNick() const { return _nick; }
  std::string getIdent() const { return _ident; }
  std::string getHost() const { return _host; }
  std::string getIp() const { return _ip; }
  int	      getLevel() const { return _level; }
};

#endif
