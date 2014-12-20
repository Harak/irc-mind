#ifndef __SOCKET_EXCEPTION_HPP__
#define __SOCKET_EXCEPTION_HPP__

#include "exception.hpp"

class SocketException : public Exception
{
public:
  SocketException(std::string s) : Exception(s) {}
};

#endif
