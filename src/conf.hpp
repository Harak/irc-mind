#ifndef __CONF_HPP__
#define __CONF_HPP__

#include <string>
#include <list>
#include <fstream>
#include <vector>

typedef std::list<std::pair<std::string, std::string> > confElem;
typedef std::vector<std::string> stringlist;

class Conf
{
  std::ifstream _file;
  
public:
  confElem	_entry;
  Conf();
  Conf(const Conf &);
  Conf		&operator=(const Conf &);

  ~Conf();
  void		mapFile();
  std::string	trim(std::string const &);
  bool		check();
  std::string	get(char const *);
  std::string	get(std::string);
  void		set(char const *, std::string const);
  void		set(std::string const, std::string const);
  bool		exists(char const *);
  bool		exists(std::string const);
};

#endif
