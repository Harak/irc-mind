#include <time.h>
#include <pthread.h>
#include "logger.hpp"
#include "exception.hpp"

extern pthread_mutex_t _mutex;

Logger::Logger(std::string const file)
  : _logfile(file)
{
  if (!open())
    throw Exception("Unable to open logfile " + _logfile);
}

Logger::Logger(const char *file)
  : _logfile(file)
{
  if (!open())
    throw Exception("Unable to open logfile " + _logfile);
}

Logger::~Logger()
{
  _output.close();
}

bool Logger::open()
{
  _output.open(_logfile.c_str(), std::ofstream::out | std::ofstream::app);

  if (_output.fail())
    return false;

  return true;
}

void Logger::setLogfile(std::string file)
{
  _logfile = file;
}

Logger &operator<<(Logger &logger, std::string str)
{
  pthread_mutex_lock(&_mutex);

  struct tm *current;
  time_t now;
  char curtime[50] = {0};
  std::string tmp;
  
  time(&now);
  current = localtime(&now);

  strftime(curtime, 50, "[ %m/%d/%Y - %T ] ", current);
  tmp = std::string(curtime);

  logger._output << tmp << str << std::endl;
  pthread_mutex_unlock(&_mutex);

  return logger;
}
