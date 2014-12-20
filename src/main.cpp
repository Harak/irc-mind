#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <list>
#include <string>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <db_cxx.h>

#include "activity.hpp"
#include "irc.hpp"
#include "user.hpp"
#include "module.hpp"
#include "conf.hpp"
#include "mind.hpp"
#include "moduleexception.hpp"
#include "confexception.hpp"
#include "module.hpp"
#include "utils.hpp"

irc			*_irc;
pthread_mutex_t		_mutex = PTHREAD_MUTEX_INITIALIZER;
void			*sigobject;

void			sighandler(int sig)
{
  Mind			*bot;

  bot = static_cast<Mind *>(sigobject);

  if (sig == SIGINT)
    {
      bot->quit();
    }
  else if (sig == SIGUSR1)
    {
      bot->rehash();
    }
}

void			*init_modules(void *arg)
{
  Mind			*m;

  m = static_cast<Mind *>(arg);

  try {
    m->initModules();
  }
  catch (Exception &e)
    {
      std::cerr << "Error during the initialization of modules: " << e.what() << std::endl;
    }

  return NULL;
}

void			*run(void *arg)
{
  Mind		*m;
  
  m = static_cast<Mind *>(arg);
  try {
      m->run();
    }
  catch (Exception &e)
    {
      std::cerr << "ERROR - " << e.what() << std::endl;
      exit(1);
    }

  return NULL;
}

void			*thread_handle(void *arg)
{
  Module		*mod;

  mod = static_cast<Module *>(arg);
  mod->run();

  return NULL;
}

Mind::Mind()
{
  std::string	admin, modload;
  
  try 
    {
      _run = true;
      _conf.mapFile();
      if (!_conf.check())
	{
	  std::cerr << "Unable to start. Please check the configuration file" << std::endl;
	  exit(1);
	}
      _logger = new Logger(_conf.get("logFile"));

      admin = _conf.get("admin");
      initAdmin(admin);

      modload = _conf.get("loadModule");
      loadModules(modload);
    }
  catch (Exception &e)
    {
      std::cerr << "Error: " << e.what() << std::endl;
      exit(1);
    }
}

void			Mind::initAdmin(std::string &admin)
{
  std::string		tok = "-";
  int			i;

  if (admin.find(" ") == std::string::npos)
    _admin.push_back(admin);
  else
    {
      for (i = 1; tok != ""; ++i)
	{
	  tok = Utils::getToken(admin, " ", i);	     
	  if (tok != "")
	    _admin.push_back(tok);
	}
    }
}

void			Mind::initModules()
{
  moduleMap::iterator	it;
  pthread_t		*thread = NULL;
  int			ret, i;

  _th = 0;
  for (it = _mod.begin(); it != _mod.end(); ++it)
    {
      it->second->setIrc(_irc);
      it->second->setConf(_conf);
      it->second->setLogger(_logger);
      try
	{
	  it->second->init();
	}
      catch (ModuleException &e)
	{
	  *_logger << "Unable to load module '" << it->second->getName() << "': " << e.what();
	  _mod.erase(it);
	  it--;
	  continue;
	}
      if (it->second->isThreaded())
	++_th;
    }

  thread = new pthread_t[_th];
  for (i = 0, it = _mod.begin(); it != _mod.end(); ++it)
    if (it->second->isThreaded())
      {
	if ((ret = pthread_create(&thread[i], NULL, thread_handle, it->second)))
	  throw Exception("Unable to create the thread for module '" + it->second->_name + "'");
	else
	  it->second->setThread(thread[i]);
	++i;
      }

  for (i = 0; i < _th; ++i)
    if ((ret = pthread_detach(thread[i])))
      throw Exception("Unable to execute the thread (" + Utils::intToString(i) + ")");
}

void			Mind::initMod(std::string &mod)
{
  Module		*m;
  pthread_t		thread;
  int			ret;

  if (_mod.find(mod) == _mod.end())
    {
      *_logger << "Warning: Module '" << mod << "' already loaded; ignoring...";
      return;
    }

  m = _mod[mod];

  m->setIrc(_irc);
  m->setConf(_conf);
  m->setLogger(_logger);

  try {
    m->init();
  }
  catch (ModuleException &e)
    {
      *_logger << "Unable to init module '" << mod << "': " << e.what();
    }

  if (m->isThreaded())
    {
      if ((ret = pthread_create(&thread, NULL, thread_handle, m)))
	throw Exception("Unable to create the thread");
      else
	m->setThread(thread);
    }

  pthread_detach(thread);
}

bool			Mind::loadMod(std::string &mod)
{
  construct		ctor;
  destruct		dtor;
  void			*handle;
  void			*sym;
  Module		*load;
  std::string		err;

  if (_mod.find(mod) != _mod.end())
    {
      *_logger << "MODULE ERROR - Module '" << mod << "' already loaded.";
      return false;
    }
  if (!(handle = dlopen(std::string("./modules/" + mod + ".so").c_str(), RTLD_NOW)))
    {
      *_logger << "MODULE ERROR - Unable to find './modules/" << mod << ".so' " << dlerror();
      return false;
    }
  if (!(sym = dlsym(handle, "create")))
    {
      *_logger << "MODULE ERROR - Cannot find constructor symbol 'create'.";
      return false;
    }
  ctor = reinterpret_cast<construct>(sym);
  load = ctor();
  if (!(sym = dlsym(handle, "destroy")))
    {
      *_logger << "MODULE ERROR - Cannot find destructor symbol 'destroy'.";
      return false;
    }

  load->setHandle(handle); // dlclose
  dtor = reinterpret_cast<destruct>(sym);
  load->setDtor(dtor); // unload
  _mod[mod] = load;
  *_logger << "MODULE - Module " << mod << " successfully loaded.";
  
  return true;
}

bool			Mind::unloadMod(std::string &mod)
{
  Module	*m;
  destruct	dtor;
  void		*handle;


  (void)handle;
  if (_mod.find(mod) == _mod.end())
    {
      *_logger << "MODULE ERROR - Unable to unload module '" << mod << "'.";
      return false;
    }
  
  m = _mod[mod];
  dtor = m->getDtor();
  handle = m->getHandle();
  dtor(m);
  _mod.erase(mod);
  //dlclose(handle); FIXME?
  *_logger << "- MODULE - Module " << mod << " successfully unloaded.";
  return true;
}

void			Mind::loadModules(std::string &modules)
{
  std::string		tmp = ".";
  int			i;

  if (modules == "")
    return;
  for (i = 1; tmp != "";)
    {
      tmp = Utils::getToken(modules, " ", i++);
      if (tmp == "")
	break;
      loadMod(tmp);
    }
  *_logger << "Modules loaded.";
}

void Mind::execAdmin(ircEvent event, std::string &data)
{
  moduleMap::iterator	it;
  std::string msg, cmd, tmp;
  std::string target;
  User	      u;

  if (event == PRIVMSG)
    {
      msg = data.substr(msg.find(":") + 2);
      msg = msg.substr(msg.find(":") + 1);
      Utils::eraseCRLF(msg);
      Utils::trim(msg);

      cmd = Utils::getToken(msg, " ", 1);
      if (cmd == "")
	cmd = msg;

      target = Utils::getToken(data, " ", 3);
      if (target[0] != '#' && target[0] != '&')
	{
	  u = irc::makeUser(data);
	  target = u.getNick();
	}
      if (cmd == ".load")
	{
	  tmp = Utils::getToken(msg, " ", 2);
	  if (loadMod(tmp))
	    _irc->privmsg(target, "Module successfully loaded.");
	  else
	    _irc->privmsg(target, "Unable to load module. Please check log file for further information.");
	  try {
	      initMod(tmp);
	  }
	  catch (Exception &e)
	    {
	      _irc->privmsg(target, "Error: Cannot init module. Unloading...");
	      unloadMod(tmp);
	    }
	}
      else if (cmd == ".unload")
	{
	  tmp = Utils::getToken(msg, " ", 2);
	  if (unloadMod(tmp))
	    _irc->privmsg(target, "Module successfully unloaded.");
	  else
	    _irc->privmsg(target, "Unable to unload module. Please check log file for further information.");
	}
      else if (cmd == ".reload")
	{
	  tmp = Utils::getToken(msg, " ", 2);
	  if (!unloadMod(tmp))
	    _irc->privmsg(target, "Unable to reload module. Please check log file for further information.");
	  if (loadMod(tmp))
	    {
	      try {
		  initMod(tmp);
	      }
	      catch (Exception &e)
		{
		  _irc->privmsg(target, "Error: Cannot init module. Unloading...");
		  unloadMod(tmp);
		} 
	      _irc->privmsg(target, "Module successfully reloaded.");
	    }
	  else
	    _irc->privmsg(target, "Unable to reload module. Please check log file for further information.");
	}
      else if (cmd == ".rehash")
	{
	  rehash();
	}
       else if (cmd == ".die")
	 {
	   std::string msg = "Ciao!";
	   _irc->quit(msg);
	   exit(0);
	 }
    }
}

void Mind::rehash()
{
  moduleMap::iterator	it;
  Conf c;
  
  try 
    {
      *_logger << "Rehashing configuration...";
      c.mapFile();
      if (!_conf.check())
	{
	  *_logger << "Configuration reload failed. Missing parameters";
	  return;
	}
      _conf = c;
      for (it = _mod.begin(); it != _mod.end(); ++it)
	it->second->setConf(_conf);
      *_logger << "Configuration successfully reloaded.";
    }
  catch (Exception &e)
    {
      *_logger << "Configuration reload failed. " << e.what();
    }
}

void Mind::quit()
{
  *_logger << "Shutting down...";
  _run = false;
}

bool Mind::isAdmin(std::string nick)
{
  stringlist::const_iterator it;

  for (it = _admin.begin(); it != _admin.end(); ++it)
    if (*it == nick)
      return true;
  return false;
}

void			Mind::run()
{
  int			i, raw;
  activityMonitor	monitor;
  struct epoll_event	*e = NULL;
  std::string		buf;
  ircEvent		event;
  User			user;
  std::string		tmp;
  moduleMap::const_iterator it;

  _irc = new irc();

  if (!_irc->connect(_conf.get("server"), atoi(_conf.get("port").c_str())))
    throw Exception("Cannot connect to server... aborting");

  _irc->setNick(_conf.get("nick"));
  _irc->setIdent(_conf.get("ident"));
  _irc->setRealname(_conf.get("realname"));

  if ((tmp = _conf.get("oper")) == "true")
    {
      _irc->setOper(true);
      _irc->setOperNick(_conf.get("operUsername"));
      _irc->setOperPass(_conf.get("operPassword"));
    }

  _irc->auth();

  if (_conf.exists("auth"))
    _irc->setNsPass(_conf.get("auth"));
  _irc->identify();

  if (_irc->isOper())
    _irc->oper();

  if (_conf.exists("autojoin"))
    {
      tmp = _conf.get("autojoin");
      _irc->join(tmp);
    }
  
  monitor.setFd(_irc->getSock()->getFd());
  monitor.setEvents(EPOLLIN | EPOLLERR | EPOLLHUP);
  if (!monitor.init())
    {
      *_logger << "Unable to init the activity monitor";
      return;
    }

  while (_run)
    if (monitor.checkActivity())
      {
	if (!(e = monitor.getActivity()))
	  {
	    _irc->end();
	    return;
	  }
	for (i = 0; i < monitor.getNb(); ++i)
	  {
	    if (e[i].events & EPOLLHUP || e[i].events & EPOLLERR)
	      {
		_irc->end();
		return;
	      }
	    if (e[i].events & EPOLLIN)
	      {		
		_irc->run();
		buf = _irc->getBuf();

		event = _irc->getEvent();
		try {
		    user = irc::makeUser(buf);
		}
		catch (Exception &e) {
		  event = UNKNOWN;
		}
		
		if (isAdmin(user.getNick()))
		  execAdmin(event, buf);

		if (isRaw(buf)) // the message is not an 'IRC event' 
		  {
		    raw = atoi(Utils::getToken(buf, " ", 2).c_str());
		    /* broadcast to every module */
		    for (it = _mod.begin(); raw != -1 && it != _mod.end(); ++it)
		      if (it->second->isEventable())
			it->second->onRaw(raw, buf);
		  }

		/* broadcast irc event to every module */
		for (it = _mod.begin(); event != UNKNOWN && it != _mod.end(); ++it)
		  if (it->second->isEventable())
		    it->second->execEvent(event, user, buf);

		event = UNKNOWN;
	      }
	  }
      }
  _irc->end();
  *_logger << "Connection closed.";
  return;
}

bool			Mind::isRaw(std::string buf)
{
  std::string		tmp;
  int			nb;

  tmp = Utils::getToken(buf, " ", 2);
  if (tmp.length() != 3)
    return false;
  nb = atoi(tmp.c_str());
  if (nb < 0)
    return false;
  return true;
}

/* 
 * Create a thread for the module initialization and launch
 * and another one for the main IRC execution
 */

void			Mind::start()
{
  pthread_t		th[2];
  int			ret;

  *_logger << "----------------------------------------------------";
  *_logger << "Starting new session";
  *_logger << "----------------------------------------------------";

  if ((ret = pthread_create(&th[0], NULL, ::run, this)))
    return;

  sleep(1); /* wait while the 'irc' class is being initialized */

  if ((ret = pthread_create(&th[1], NULL, ::init_modules, this)))
    return;

  if ((ret = pthread_join(th[0], NULL)))
    return;
  if ((ret = pthread_join(th[1], NULL)))
    return;
 }

int			main()
{
  pid_t			pid;
  pid_t			sid;
  int			fd;
  Mind			bot;
  char			*strpid;
        
  pid = fork();
  if (pid < 0)
    exit(EXIT_FAILURE);

  if (pid > 0) 
    exit(EXIT_SUCCESS);
  
  umask(0);
                
  sid = setsid();
  if (sid < 0) 
    exit(EXIT_FAILURE);
        
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  if ((fd = open("./mind.pid", O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0)
    exit(EXIT_FAILURE);

  strpid = const_cast<char *>(Utils::intToString(getpid()).c_str());
  
  write(fd, strpid, strlen(strpid));
  close(fd);

  sigobject = &bot;
  signal(SIGUSR1, sighandler);
  signal(SIGINT, sighandler);

  bot.start();
  exit(EXIT_SUCCESS);

  return 0;
}
