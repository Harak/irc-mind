#ifndef __CONF_EXCEPTION_HPP_
#define __CONF_EXCEPTION_HPP_

#include "exception.hpp"

class ConfException : public Exception
{
public:
  ConfException(std::string s) : Exception(s) {}
};

#endif
