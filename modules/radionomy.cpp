#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <db_cxx.h>
#include "moduleexception.hpp"
#include "radionomy.hpp"

static size_t		my_read(void *ptr, size_t sz, size_t nb, void *stream)
{
  return fwrite(ptr, sz, nb, (FILE*)stream);
}

void		Radionomy::setChannel(std::string channel)
{
  _channel = channel;
}

Radionomy::Radionomy()
  : Module("radionomy")
{
  _artist = "";
  _title = "";
}

Radionomy::~Radionomy()
{
  updateAnimlist();

  if (_db)
    {
      _db->close(0);
      delete _db;
      _db = NULL;
    }
  if (_env)
    {
      _env->close(0);
      delete _env;
      _env = NULL;
    }
}

void		Radionomy::getXml()
{
  CURL		*curl_handle;
  FILE		*fd;

  if (!(fd = fopen(_filename.c_str(), "w")))
    return;
  fputs("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n", fd);
  fputs("<!DOCTYPE currentsong>\n", fd);
  fputs("<currentsong>\n", fd);

  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();

  curl_easy_setopt(curl_handle, CURLOPT_URL, _url.c_str());
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &my_read);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fd);
  curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5);
  curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 3);
  curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 1800);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "mind/0.1");
  curl_easy_perform(curl_handle);

  curl_easy_cleanup(curl_handle);

  curl_global_cleanup();

  fputs("</currentsong>\n", fd);
  fclose(fd);
}

std::string	Radionomy::getElem(char const *name)
{
  xmlNodePtr	node, elem, nelem;
  xmlChar	*value;
  std::string	ret = "";

  if (!(_doc = xmlParseFile(_filename.c_str())))
    return ret;
  if (!(node =  xmlDocGetRootElement(_doc)))
    {
      xmlFreeDoc(_doc);
      return ret;
    }
  for (elem = node->xmlChildrenNode; elem; elem = elem->next)
    if (!strcmp((char*)elem->name, "tracks"))
      {
	node = elem;
	break;
      }
  for (elem = node->xmlChildrenNode; elem; elem = elem->next)
    {
      if (!strcmp((char*)elem->name, "track"))
	for (nelem = elem->xmlChildrenNode; nelem; nelem = nelem->next)
	  if (!strcmp((char*)nelem->name, name))
	    {
	      value = xmlNodeListGetString(_doc, nelem->xmlChildrenNode, 1);
	      if (!value)
		{
		  xmlFreeDoc(_doc);
		  return ret;
		}
	      ret = std::string((char*)value);
	      xmlFree(value);
	      break;
	    }
    }
  xmlFreeDoc(_doc);
  return ret;
}

void		Radionomy::run()
{
  getAnimlist();
  _irc->join(_channel);
  while (1)
    {
      sleep(7);
      getCurrentSong(false);
    }
}

void		Radionomy::getCurrentSong(bool force)
{
  std::string	duration("");
  std::string	tmp;
  std::string	title("1");
  std::string	artist("1");

  getXml();
  tmp = getElem("starttime");
  title = getElem("title");
  if (title.substr(0, 2) == "- ")
    title.erase(0, 2);
  artist = getElem("artists");
  if (force)
    {	  
      if (title.substr(0, 6) == "Advert" && artist.substr(0, 6) == "Advert")
	_irc->privmsg(getChannel(), "\002\00304" + _name + "\017 diffuse une annonce.");
      else
	_irc->privmsg(getChannel(), "\002\00304" + _name + "\017 diffuse en ce moment le \002titre\017 : \00310" + title + "\017, \002Artiste(s)\017 : \00302" + artist);
      duration = tmp;
      _artist = artist;
      _title = title;
    }
  else if (duration != tmp && (_artist != artist && _title != title))
    {
      if (title != "" && artist != "" && artist != "Content")
	{
	  if (title.substr(0, 6) == "Advert" && artist.substr(0, 6) == "Advert")
	    return;
	  _irc->privmsg(getChannel(), "\002\00304" + _name + "\017 diffuse en ce moment le \002titre\017 : \00310" + title + "\017, \002Artiste(s)\017 : \00302" + artist);
	  duration = tmp;
	  _artist = artist;
	  _title = title;
	}
    }
}

void		Radionomy::init()
{
  std::string	tmp;
  std::string   anim;
  
  _threaded = true;
  _event = true;

  if (!_conf.exists("radionomyName"))
    throw ModuleException("Missing conf parameter 'radionomyName'");
  _name = _conf.get("radionomyName");

  if (!_conf.exists("radionomyUid") || !_conf.exists("radionomyApiKey"))
    throw ModuleException("Make sure that you provide Radio UID and API Key");
  _url = "http://api.radionomy.com/currentsong.cfm?radiouid=" + _conf.get("radionomyUid");
  _url += "&type=xml&apikey=" + _conf.get("radionomyApiKey");

  if (!_conf.exists("radionomyChannel"))
    throw ModuleException("Missing conf parameter 'radionomyChannel'");    
  _channel = _conf.get("radionomyChannel");

  if (!_conf.exists("radionomyCmd"))
    throw ModuleException("Missing conf parameter 'radionomyCmd'");
  _cmd = _conf.get("radionomyCmd");

  if (!_conf.exists("radionomyAdmin"))
    throw ModuleException("Missing conf parameter 'radionomAdmin'");
  _admin = _conf.get("radionomyAdmin");
  Utils::trim(_admin);
  
  Utils::trim(tmp);

  if (!_conf.exists("radionomyXml"))
    _filename = "/tmp/currentsong.xml";
  else
    _filename = _conf.get("radionomyXml");

  _irc->privmsg("OperServ", "SET SUPERADMIN ON");

  try 
    {
      _env = new DbEnv(0);
      _env->open("./db/", DB_CREATE | DB_INIT_MPOOL, 0);
      
      _db = new Db(_env, 0);
      if (_db->open(NULL, "radionomy.db", NULL, DB_HASH, DB_CREATE, 0))
	{
	  *_logger << "Unable to open radionomy.db properly";
	  _db->close(0);
	}
    }
  catch (DbException &e)
    {
      *_logger << "Unable to create DB for radionomy module: " << e.what();
    }
  _suffix = getSuffix();
}

void		Radionomy::onPrivmsg(User &u, std::string s)
{
  size_t	pos;
  std::string	target;
  std::string	msg;
  std::string	cmd;
  std::string	tmp;
  std::string	args;
  std::vector<std::string>::iterator it;
  int		i;

  tmp = s;
  Utils::stripColors(tmp);
  if (s[0] == '#')
    {
      if ((pos = s.find(" ")) == std::string::npos)
	return;
      target = s.substr(0, pos);
    }
  else
    target = u.getNick();
  if ((pos = s.find(":")) == std::string::npos)
    return;
  msg = s.substr(pos + 1, s.length() - pos);
  Utils::eraseCRLF(msg);

  if ((pos = msg.find(" ")) == std::string::npos)
    {
      cmd = msg;
      args = "";
    }
  else
    {
      cmd = msg.substr(0, pos);
      args = msg.substr(pos + 1);
    }
  Utils::trim(args);
  Utils::stripColors(cmd);

  if (cmd == _cmd && target == _channel)
    getCurrentSong(true);
  else if (cmd == "!off" && isAdmin(u.getNick()) && _anim != "")
    {
      i = _anim.rfind(_suffix);
      tmp = getAnnounce(2);
      if (tmp != "")
	{
	  if (tmp.find("%anim%") != std::string::npos)
	    tmp.replace(tmp.find("%anim%"), 6, _anim);
	  _irc->privmsg(_channel, tmp);
	}
      else
	_irc->privmsg(_channel, _anim + " n'est plus à l'antenne.");
      _irc->privmsg("OperServ", "SVSNICK " + _anim + " " + _anim.erase(i));
      _anim = "";
    }
  else if (Utils::toLower(cmd) == "!onair")
    {
      it = std::find(_animlist.begin(), _animlist.end(), u.getNick());
      if (it == _animlist.end())
	{
	  _irc->notice(u.getNick(), "Vous devez être dans la liste des animateurs pour pouvoir utiliser cette commande");
	  return;
	}
      if (_anim != "")
	_irc->notice(u.getNick(), "Il y a déjà une personne qui anime (" + _anim + ").");
      else
	{	  
	  _irc->privmsg("OperServ", "SVSNICK " + u.getNick() + " " + u.getNick() + _suffix);
	  _anim = u.getNick() + _suffix;
	  tmp = getAnnounce(1);
	  if (tmp != "")
	    {
	      if (tmp.find("%anim%") != std::string::npos)
		tmp.replace(tmp.find("%anim%"), 6, _anim);
	      _irc->privmsg(_channel, tmp);
	    }
	  else
	    _irc->privmsg(_channel, _anim + " passe à l'antenne. Bonne anim' !");
	}
    }
  else if (Utils::toLower(cmd) == "!offair")
    {
      if (u.getNick() == _anim)
	{
	  i = _anim.rfind(_suffix);
	  tmp = getAnnounce(2);
	  if (tmp != "")
	    {
	      if (tmp.find("%anim%") != std::string::npos)
		tmp.replace(tmp.find("%anim%"), 6, _anim);
	      _irc->privmsg(_channel, tmp);
	    }
	  else
	    _irc->privmsg(_channel, _anim + " n'est plus à l'antenne.");
	  _irc->privmsg("OperServ", "SVSNICK " + u.getNick() + " " + _anim.erase(i));
	  _anim = "";
	}
    }
  else if (cmd == "!dedi" || cmd == "$_DEDICACE_$")
    {
      if (_anim == "")
	_irc->notice(u.getNick(), "Aucun animateur n'est à l'antenne pour le moment !");
      else
	{
	  if (args == "")
	    _irc->notice(u.getNick(), "Tu as oublié ta dédicace : !dedi Ta dédicace");
	  else
	    {
	      _irc->privmsg(_anim, "Dédicace de " + u.getNick() + " : " + args);
	      _irc->notice(u.getNick(), "Ta dédicace a été prise en compte !");
	    }
	}
    }
  else if (cmd == "!suffixe")
    {
      if (isAdmin(u.getNick()))
	{
	  if (args.find(" ") != std::string::npos)
	    tmp = Utils::getToken(args, " ", 1);
	  else
	    tmp = args;
	  if (tmp != "")
	  {
	    _suffix = tmp;
	    if (setSuffix(tmp))
	      _irc->notice(u.getNick(), "Modification effectuée.");
	    else
	      _irc->notice(u.getNick(), "Impossible de modifier le suffixe.");
	  }
	  else
	    {
	      _irc->notice(u.getNick(), "Suffixe actuel : " + _suffix);
	      _irc->notice(u.getNick(), "Syntaxe pour modifier le suffixe : !suffixe |OnAir");
	    }
	}
      else
	_irc->notice(u.getNick(), "Accès refusé.");
    }
  else if (cmd == "!anim")
    {
      if (isAdmin(u.getNick()))
	{
	  tmp = Utils::getToken(args, " ", 1);
	  if (tmp == "add" || tmp == "del")
	    {
	      args = args.substr(args.find(" ") + 1);
	      if (tmp == "add")
		{
		  if (addAnim(args))
		    _irc->notice(u.getNick(), "Ajouté avec succès.");
		  else
		    _irc->notice(u.getNick(), "Erreur : Non ajouté.");
		}
	      else
		{
		  if (delAnim(args))
		    _irc->notice(u.getNick(), "Supprimé avec succès.");
		  else
		    _irc->notice(u.getNick(), "Erreur : Non supprimé.");
		}
	    }
	  else
	    _irc->notice(u.getNick(), "Syntaxe : !anim <add/del> pseudo");
	  updateAnimlist();
	}
      else
	_irc->notice(u.getNick(), "Accès refusé.");
    }
  else if (cmd == "!animlist")
    {
      stringlist::iterator it;
      if (isAdmin(u.getNick()))
	{
	  if (_animlist.size() != 0)
	    {
	      _irc->notice(u.getNick(), "Voici la liste des animateurs :");
	      for (it = _animlist.begin(); it != _animlist.end(); ++it)
		_irc->notice(u.getNick(), " - " + *it);
	    }
	  else
	    _irc->notice(u.getNick(), "La liste des animateurs est vide.");
	}
      else
	_irc->notice(u.getNick(), "Accès refusé.");
    }
  else if (cmd == "!annonce")
    {
      if (isAdmin(u.getNick()))
	{
	  tmp = Utils::getToken(args, " ", 1);
	  if (tmp == "on" || tmp == "off")
	    args = args.substr(args.find(" ") + 1);
	  if (setAnnounce(args, tmp == "on" ? 1 : (tmp == "off" ? 2 : 3)))
	    _irc->notice(u.getNick(), "Changement effectué");
	  else
	    _irc->notice(u.getNick(), "Erreur : Impossible de mettre à jour la base.");
	}
      else
	_irc->notice(u.getNick(), "Accès refusé.");
    }
  _db->sync(0);
}

void		Radionomy::onNick(User &u, std::string nick)
{
  std::string tmp;

  (void)nick;
  if (u.getNick() == _anim)
    {
      tmp = getAnnounce(2);
      if (tmp != "")
	{
	  if (tmp.find("%anim%") != std::string::npos)
	    tmp.replace(tmp.find("%anim%"), 6, _anim);
	  _irc->privmsg(_channel, tmp);
	}
      else
	_irc->privmsg(_channel, _anim + " n'est plus à l'antenne.");
      _anim = "";
    }
}

bool		Radionomy::addAnim(std::string anim)
{
  stringlist::iterator it;

  for (it = _animlist.begin(); it != _animlist.end(); ++it)
    if (*it == anim)
      return true;

  _animlist.push_back(anim);

  return true;
}

bool		Radionomy::delAnim(std::string anim)
{
  stringlist::iterator it;

  for (it = _animlist.begin(); it != _animlist.end(); ++it)
    if (*it == anim)
      {
	_animlist.erase(it);
	return true;
      }

  return true;
}

bool		Radionomy::setSuffix(std::string suffix)
{
  std::string tmp = "suffix";
  Dbt key(const_cast<char*>(tmp.data()), tmp.size());
  Dbt data(const_cast<char*>(suffix.data()), suffix.size());

  try 
    {
      _db->put(NULL, &key, &data, 0);
    }
  catch (DbException &e)
    {
      *_logger << "Unable to set suffix: " << e.what();
      return false;
    }
  return true;
}

std::string		Radionomy::getSuffix()
{
  std::string	tmp = "suffix";
  char		buf[512] = {0};
  Dbt		key(const_cast<char*>(tmp.data()), tmp.size());
  Dbt		data;

  data.set_data(buf);
  data.set_ulen(512);
  data.set_flags(DB_DBT_USERMEM);

  try 
    {
      _db->get(NULL, &key, &data, 0);
    }
  catch (DbException &e)
    {
      *_logger << "Unable to get suffix: " << e.what();
      return std::string("|OnAir");
    }
  tmp = std::string(buf);
  if (tmp == "")
    return std::string("|OnAir");
  return tmp;
}

bool		Radionomy::setAnnounce(std::string announce, char type)
{
  std::string	t;

  if (type == 1)
    t = "announce_on";
  else if (type == 2)
    t = "announce_off";
  else
    return false;

  Dbt key(const_cast<char *>(t.data()), t.size());
  Dbt data(const_cast<char *>(announce.data()), announce.size() + 1);
  if (_db)
    {
      _db->put(NULL, &key, &data, 0);
      return true;
    }
  return false;
}

std::string    	Radionomy::getAnnounce(char type)
{
  std::string	tmp;
  char		buf[512] = {0};

  if (type == 1)
    tmp = "announce_on";
  else if (type == 2)
    tmp = "announce_off";
  else
    tmp = "";

  Dbt		key(const_cast<char*>(tmp.data()), tmp.size());
  Dbt		data;

  data.set_data(buf);
  data.set_ulen(512);
  data.set_flags(DB_DBT_USERMEM);

  _db->get(NULL, &key, &data, 0);

  return std::string(buf);
}

void		Radionomy::getAnimlist()
{
  std::string	k("animlist");
  std::string	tmp, tok;
  int		i;
  char		buf[512] = {0};

  Dbt key(const_cast<char*>(k.data()), k.size());  
  Dbt data;

  data.set_data(buf);
  data.set_ulen(512);
  data.set_flags(DB_DBT_USERMEM);

  _db->get(NULL, &key, &data, 0);

  _animlist.clear();

  tmp = std::string(buf);
  if (tmp == "")
    return;
  if (tmp.find(" ") == std::string::npos)
    _animlist.push_back(tmp);
  else
    {
      tok = "-";
      for (i = 1; tok != ""; ++i)
	{
	  tok = Utils::getToken(tmp, " ", i);
	  if (tok != "")
	    _animlist.push_back(tok);
	}
    }
}

void		Radionomy::updateAnimlist()
{
  std::string k("animlist");
  std::string	anim = "";
  stringlist::iterator it;

  for (it = _animlist.begin(); it != _animlist.end(); ++it)
    anim += " " + *it;
  
  Utils::trim(anim);

  Dbt	key(const_cast<char*>(k.data()), k.size());
  Dbt	data(const_cast<char*>(anim.data()), anim.size() + 1);

  _db->put(NULL, &key, &data, 0);
}

bool		Radionomy::isAdmin(std::string nick)
{
  int		i;
  std::string	tok = "-";

  nick = Utils::toLower(nick);
  for (i = 1; tok != ""; ++i)
    {
      tok = Utils::getToken(_admin, " ", i);
      if (Utils::toLower(tok) == nick)
	return true;
    }
  return false;
}

extern "C"
{
  Module	*create()
  {
    return new Radionomy;
  }
  void		destroy(Module *m)
  {
    delete m;
  }
}
