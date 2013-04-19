#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <string>
#include <fstream>

class Logger
{
  std::string	_logfile;
  
public:
  std::ofstream _output;

  Logger(std::string const);
  Logger(const char *);
  ~Logger();

  bool open();
  void setLogfile(std::string);

};

Logger &operator<<(Logger &, std::string);

#endif
