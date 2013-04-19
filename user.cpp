#include "user.hpp"

void User::setNick(std::string const nick)
{
  _nick = nick;
}

void User::setIp(std::string const ip)
{
  _ip = ip;
}

void User::setHost(std::string const host)
{
  _host = host;
}

void User::setIdent(std::string const ident)
{
  _ident = ident;
}

void User::setLevel(int const level)
{
  _level = level;
}

