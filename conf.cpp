#include <stdlib.h>
#include <string>
#include <iostream>
#include "conf.hpp"

Conf::Conf()
{
  _file.open("mind.conf", std::ifstream::in);
}

Conf::Conf(const Conf &c)
{
  _file.open("mind.conf", std::ifstream::in);
  _entry = c._entry;
}

Conf& Conf::operator=(const Conf &c)
{
  _file.open("mind.conf", std::ifstream::in);

  confElem::const_iterator it;

  for (it = c._entry.begin(); it != c._entry.end(); ++it)
    _entry.push_back(*it);

  return *this;
}

Conf::~Conf()
{
  if (_file.is_open())
    _file.close();
}

void Conf::mapFile()
{
  char		buf[256];
  std::string	tmp;
  std::string	key, value;
  int		nb = 0;
  size_t	eq;

  if (!_file.good())
    {
      std::cerr << "Unable to read configuration file." << std::endl;
      return;
    }
  do
    {
      ++nb;
      _file.getline(buf, 256);
      tmp = trim(std::string(buf));

      /* Ignoring comments and empty lines */
      if (tmp[0] == '#' || tmp == "")
	continue;

      /* Syntax error key=value expected */
      eq = tmp.find("=");
      if (eq == std::string::npos || eq+1 == std::string::npos)
	{
	  std::cerr << "Syntax error: line " << nb;
	  std::cerr << ": Expected 'key=value'" << std::endl;
	  return;
	}

      /* Get key and value in two different variable strings */
      key = tmp.substr(0, eq);
      value = tmp.substr(eq+1);

      /* Add the key/value in the list */
      if (exists(key))
	std::cerr << "Warning: ignoring line " << nb << ": already set" << std::endl;
      else
	set(key, value);
      
    } while (!_file.fail() && !_file.eof());
}

std::string	Conf::trim(std::string const& source)
{
  char const	*delims = " \t\r\n\b";
  std::string	result(source);
  std::string::size_type index = result.find_last_not_of(delims);

  if (index != std::string::npos)
    result.erase(++index);

  index = result.find_first_not_of(delims);
  if (index != std::string::npos)
    result.erase(0, index);
  else
    result.erase();

  return result;
}

bool Conf::check()
{
  std::string mandatory[] = {
    "server", "port", "ident", "nick", "realname", "logFile", "admin", "oper"
  };
  int	      i;

  for (i = 0; i < 8; ++i)
    if (get(mandatory[i].c_str()) == "")
      {
	std::cerr << "Configuration file: You must have a '" << mandatory[i] << "' parameter" << std::endl;
	return false;
      }
  return true;
}

std::string Conf::get(char const *key)
{
  confElem::iterator it;
 
  for (it = _entry.begin(); it != _entry.end(); it++)
    if (std::string(key) == std::string(it->first))
      return it->second;
  return std::string("");
}

std::string Conf::get(std::string key)
{
  return get(key.c_str());
}

bool Conf::exists(char const *key)
{
  return exists(std::string(key));
}

bool Conf::exists(std::string const key)
{
  confElem::iterator it;
  
  for (it = _entry.begin(); it != _entry.end(); it++)
    if (std::string(it->first) == key)
      return true;
  return false;
}

void Conf::set(char const *key, std::string const value)
{
  set(std::string(key), value);
}

void Conf::set(std::string const key, std::string const value)
{
  _entry.push_front(make_pair(key, value));
}
