#ifndef __MODULE_EXCEPTION_HPP_
#define __MODULE_EXCEPTION_HPP_

#include "exception.hpp"

class ModuleException : public Exception
{
public:
  ModuleException(std::string s) : Exception(s) {}
};

#endif
