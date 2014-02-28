#ifndef __UTILS__HPP__
#define __UTILS__HPP__

#include <vector>
#include <string>

typedef std::vector<std::string> stringlist;

class Utils
{
public:
  
  static std::string	getToken(std::string &, char const *, int const);
  static std::string	getToken(std::string &, std::string &, int const);
  static void		eraseCRLF(std::string &);
  static std::string	toUpper(std::string);
  static std::string	toLower(std::string);
  static void		trim(std::string &);
  static std::string	intToString(int);
  static int		stringToInt(std::string);
  static void		stripColors(std::string &);
  static std::string	md5(std::string);
  static std::string	intToHexStr(int);
};
#endif
