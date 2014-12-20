#ifndef __RADIONOMY_HPP__
#define __RADIONOMY_HPP__

#include <string>
#include <vector>
#include <libxml/xmlreader.h>
#include <db_cxx.h>
#include "utils.hpp"
#include "module.hpp"

void *radionomy_run(void *);

class Radionomy : public Module
{
  DbEnv		*_env;
  Db		*_db;

  std::string _name;
  std::string _cmd;
  std::string _listen;
  std::string _site;
  std::string _filename;
  xmlDocPtr   _doc;
  std::string _url;
  std::string _channel;
  std::string _admin;

  stringlist _animlist;
  std::string _anim;
  std::string _suffix;

  bool		addAnim(std::string);
  bool		delAnim(std::string);
  bool		setAnnounce(std::string, char);
  std::string	getAnnounce(char);
  bool		setSuffix(std::string);
  std::string	getSuffix();

  void		getAnimlist();
  void		updateAnimlist();

  bool		isAdmin(std::string);

public:
  std::string _artist;
  std::string _title;

  Radionomy();
  virtual ~Radionomy();

  std::string	getElem(char const *);
  void		getXml();
  void		setUrl(std::string url) { _url = url; }
  void		setChannel(std::string);
  std::string	getChannel(){ return _channel; }
  void		getCurrentSong(bool);
  void		run();

  void		init();

  void		onPrivmsg(User &, std::string);
  void		onNick(User &, std::string);
};

#endif
