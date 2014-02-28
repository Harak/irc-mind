#include <openssl/md5.h>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <string>
#include "utils.hpp"

std::string	Utils::getToken(std::string &haystack, char const *needle, int const idx)
{
  std::string ndl = needle;

  return getToken(haystack, ndl, idx);
}

std::string	Utils::getToken(std::string &haystack, std::string &needle, int const idx)
{
  size_t        p;
  std::string   tmp;
  int           i;

  tmp = std::string(haystack);
  if (idx == 1)
    {
      if ((p = tmp.find(needle)) == std::string::npos)
        return std::string("");
      return tmp.substr(0, p);
    }
  for (i = 0; i < idx-1; ++i)
    {
      if ((p = tmp.find(needle)) == std::string::npos)
        return std::string("");
      tmp = tmp.substr(p+1);
    }
  return tmp.substr(0, tmp.find(needle));
}

void		Utils::eraseCRLF(std::string &str)
{
  size_t	pos;

  if ((pos = str.find("\r")) != std::string::npos)
    str.erase(pos);
  if ((pos = str.find("\n")) != std::string::npos)
    str.erase(pos);

}

std::string	Utils::toUpper(std::string str)
{
  std::string	upper = str;
  std::string::iterator it;

  for (it = upper.begin(); it != upper.end(); ++it)
    *it = ::toupper(*it);
  
  return upper;
}

std::string	Utils::toLower(std::string str)
{
  std::string	upper = str;
  std::string::iterator it;

  for (it = upper.begin(); it != upper.end(); ++it)
    *it = ::tolower(*it);
  
  return upper;
}

void Utils::trim(std::string& str)
{
  std::string::size_type pos = str.find_last_not_of(' ');
  
  if (pos != std::string::npos) 
    {
      str.erase(pos + 1);
      pos = str.find_first_not_of(' ');
      if (pos != std::string::npos) 
	str.erase(0, pos);
    }
  else 
    str.erase(str.begin(), str.end());
}

std::string Utils::intToString(int number)
{
  std::stringstream ss; 

  ss << number;
  return ss.str();
}

int Utils::stringToInt(std::string str)
{
  return atoi(str.c_str());
}

void Utils::stripColors(std::string &str)
{
  size_t	pos;

  while ((pos = str.find("\x03")) != std::string::npos)
    {
      if (isdigit(str[pos + 1]))
	{
	  if (isdigit(str[pos + 2]))
	    str.erase(pos, 3);
	  else
	    str.erase(pos, 2);
	}
      if (str[pos + 1] == ',')
	{
	  if (isdigit(str[pos + 2]) && isdigit(str[pos + 3]))
	    str.erase(pos, 4);
	  else if (isdigit(str[pos + 2]))
	    str.erase(pos, 3);
	}
      if (str[pos] == '\x03')
	str.erase(pos, 1);
    }
  std::remove(str.begin(), str.end(), '\x02');
  std::remove(str.begin(), str.end(), '\x03');
  std::remove(str.begin(), str.end(), '\x0f');
  std::remove(str.begin(), str.end(), '\x15');
  std::remove(str.begin(), str.end(), '\x16');
}

std::string Utils::md5(std::string value)
{
  unsigned char		dest[MD5_DIGEST_LENGTH];
  const unsigned char	*val = reinterpret_cast<const unsigned char*>(value.c_str());
  int			i;
  std::string		hash = "";
  std::string		tmp;

  MD5(val, value.size(), dest);

  for (i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
      tmp = Utils::intToHexStr(dest[i]);
      hash += (dest[i] <= 0x0f ? "0" + tmp : tmp);
    }

  return hash;
}

std::string Utils::intToHexStr(int n)
{
  std::stringstream ss; 

  ss << std::hex << n;
  return ss.str();
}
