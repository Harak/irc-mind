#ifndef __SOCKET_EXCEPTION_HPP_
#define __SOCKET_EXCEPTION_HPP_

#include "exception.hpp"

class SocketException : public Exception
{
public:
  SocketException(std::string s) : Exception(s) {}
};

#endif
