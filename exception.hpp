#ifndef __EXCEPTION_HPP__
#define __EXCEPTION_HPP__

#include <string>
#include <stdexcept>

class Exception : std::exception
{
  std::string _msg;
public:

  Exception(std::string m) : _msg(m) {}
  ~Exception() throw() {}
  const char *what() const throw() { return _msg.c_str(); }
};

#endif
