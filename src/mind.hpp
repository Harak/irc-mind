#ifndef __MIND_HPP__
#define __MIND_HPP__

#include <string>
#include <list>
#include "conf.hpp"
#include "logger.hpp"

#include "module.hpp"

class Mind
{
  moduleMap  _mod;
  int	     _th;
  Conf	     _conf;
  stringlist _admin;
  Logger     *_logger;

  void loadModules(std::string &);
  bool loadMod(std::string &);
  bool unloadMod(std::string &);
  bool isAdmin(std::string);
  void execAdmin(ircEvent, std::string &);
  bool isRaw(std::string);

public:
  Mind();
  ~Mind(){}

  void run();
  void start();
  void initMod(std::string &);
  void initModules();
  void initAdmin(std::string &);
};

#endif
