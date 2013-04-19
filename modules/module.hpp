#ifndef __MODULE_HPP__
#define __MODULE_HPP__

#include <functional>
#include <string>
#include <list>
#include <map>
#include "utils.hpp"
#include "conf.hpp"
#include "irc.hpp"
#include "logger.hpp"
#include "user.hpp"

class Module;

typedef std::map<std::string, Module *> moduleMap;
typedef Module *(*construct)(void);
typedef void (*destruct)(Module *);

class Module
{
  std::map<ircEvent, void (Module::*)(User &, std::string)> singleEventMap;
  std::map<ircEvent, void (Module::*)(User &, std::string, std::string)> doubleEventMap;

  destruct	_dtor;
  void		*_handle;
  pthread_t	_thread;

protected:
  Conf		_conf;
  Logger	*_logger;

  bool		_loaded;
  bool		_threaded;
  bool		_event;
  irc		*_irc;

public:
  std::string	_name;

  void setHandle(void *handle) { _handle = handle; }
  void *getHandle() { return _handle; }
  void setDtor(destruct dtor) { _dtor = dtor; }
  destruct getDtor() { return _dtor; }

  Module(std::string s) : _name(s) 
  {
    _loaded = true;
    _thread = (pthread_t)-1;
    singleEventMap[PRIVMSG] = &Module::onPrivmsg;
    singleEventMap[NOTICE] = &Module::onNotice;
    singleEventMap[JOIN] = &Module::onJoin;
    singleEventMap[QUIT] = &Module::onQuit;
    singleEventMap[NICK] = &Module::onNick;
    doubleEventMap[KICK] = &Module::onKick;
    doubleEventMap[MODE] = &Module::onMode;
    doubleEventMap[PART] = &Module::onPart;
  }
  virtual ~Module()
  {
    if (isThreaded() && _thread != (pthread_t)-1)
      pthread_cancel(_thread);
  }
  
  virtual void	onPrivmsg(User &, std::string){}
  virtual void	onNotice(User &, std::string){}
  virtual void	onJoin(User &, std::string){}
  virtual void	onQuit(User &, std::string){}
  virtual void	onNick(User &, std::string){}
  virtual void	onKick(User &, std::string, std::string){}
  virtual void	onMode(User &, std::string, std::string){}
  virtual void	onPart(User &, std::string, std::string){}
  virtual void	onRaw(int, std::string){}

  virtual void	init() = 0;
  virtual void	run() = 0;
  
  void		execEvent(ircEvent &e, User &u, std::string &buf) 
  {
    std::string arg1, arg2;
    size_t	pos;

    if ((pos = buf.find(" ")) == std::string::npos)
      return;
    arg1 = buf.substr(pos + 1);
    if ((pos = arg1.find(" ")) == std::string::npos)
      return;
    arg1 = arg1.substr(pos + 1);

    if ((pos = arg1.find(" ")) != std::string::npos)
      arg2 = arg1.substr(pos + 1);

    if (e == KICK || e == MODE || e == PART)
      (this->*doubleEventMap[e])(u, arg1, arg2);
    else
      (this->*singleEventMap[e])(u, arg1);
  }

  bool		isThreaded() const { return _threaded; }
  bool		isEventable() const { return _event; }

  void		setIrc(irc *irc){ _irc = irc; }
  void		setConf(Conf conf){ _conf = conf; }
  void		setLogger(Logger *logger){ _logger = logger; }
  std::string	getName() const { return _name; }
  void		setThread(pthread_t t) { _thread = t; }
};

#endif
