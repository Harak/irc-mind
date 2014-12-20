#include	<vector>
#include	<string>
#include	<iostream>
#include	"exception.hpp"
#include       	"socket.hpp"
#include	"utils.hpp"
#include	"irc.hpp"

irc::irc()
  : _port(6667), _server(""), _nick("Mind"), _anick("Mind`"), 
    _ident("mind"), _realname("Mind v0.1 - IRC Bot"), _nickserv("NickServ"), _opernick("Mind")
{
  _nspass = "";
  _operpass = "";
  _isOper = false;
  if (action.empty())
    {
      action.push_back(std::make_pair(std::string("433"), &irc::anick));
      action.push_back(std::make_pair(std::string("451"), &irc::anick));
      action.push_back(std::make_pair(std::string("381"), &irc::isOper));
      action.push_back(std::make_pair(std::string("001"), &irc::oper));
    }
}

irc::~irc()
{
  if (_sock)
    delete _sock;
}

bool irc::connect(std::string host, int port)
{
  _port = port;
  _server = host;
  _sock = new clientSocket(host, port);
  return _sock->connect();
}

bool irc::connect(char const *host, int port)
{
  return connect(std::string(host), port);
}

bool irc::connect(std::string host)
{
  return connect(host, 6667);
}

bool irc::connect(char const *host)
{
  return connect(host, 6667);
}
void irc::end()
{
  _sock->close();
}

bool irc::anick()
{
  _sock->send("NICK " + _anick + "\r\n");
  return true;
}

void irc::setNick(char const *nick)
{
  setNick(std::string(nick));
}

void irc::setNick(std::string const nick)
{
  _nick = nick;
  _anick = _nick + "`";
}

void irc::setIdent(char const *ident)
{
  setIdent(std::string(ident));
}

void irc::setIdent(std::string ident)
{
  _ident = ident;
}

void irc::setRealname(char const *realname)
{
  setRealname(std::string(realname));
}

void irc::setRealname(std::string const realname)
{
  _realname = realname;
}

void irc::setServer(std::string const server)
{
  _server = server;
}

void irc::setServer(char const *server)
{
  setServer(std::string(server));
}

clientSocket *irc::getSock() const
{
  return _sock;
}

bool irc::auth()
{
  nick();
  user();

  
  return true;
}

bool irc::checkActions()
{
  std::string msg = Utils::getToken(_buf, " ", 2);
  std::vector<ircExec>::const_iterator b;
  bool flag = false;

  for (b = action.begin(); b != action.end(); ++b)
    {
      if (b->first == msg)
	{
	  flag = true;
	  (this->*b->second)();
	}
    }
  return flag;
}

void irc::setNsPass(std::string const pass)
{
  _nspass = pass;
}

void irc::setNsPass(char const *pass)
{
  _nspass = std::string(pass);
}

void irc::setNsNick(std::string const nick)
{
  _nickserv = nick;
}

void irc::setNsNick(char const *nick)
{
  _nickserv = std::string(nick);
}

void irc::setOper(bool f)
{
  _isOper = f;
}

void irc::setOperNick(std::string const nick)
{
  _opernick = nick;
}

void irc::setOperNick(char const *nick)
{
  _opernick = std::string(nick);
}

void irc::setOperPass(std::string const pass)
{
  _operpass = pass;
}

void irc::setOperPass(char const *pass)
{
  _operpass = std::string(pass);
}

bool irc::isOper()
{
  return _isOper;
}

// IRC (RFC 1459) functions

void irc::user()
{
  _sock->send("USER " + _nick + " " + _ident + " whocares :" + _realname + "\r\n");
}

void irc::user(std::string &nick, std::string &ident, std::string &realname)
{
  _sock->send("USER " + nick + " " + ident + " whocares :" + realname + "\r\n");
}

void irc::nick(std::string &nick)
{
  _sock->send("NICK " + nick + "\r\n");
}

void irc::nick()
{
  _sock->send("NICK " + _nick + "\r\n");
}

void irc::quit(std::string &msg)
{
  if (msg != "")
    _sock->send("QUIT :" + msg + "\r\n");
  else
    _sock->send("QUIT :" + std::string(DEFAULT_QUIT_MSG)  + "\r\n");
}

void irc::privmsg(std::string target, std::string msg)
{
  _sock->send("PRIVMSG " + target + " :" + msg  + "\r\n");
}

void irc::join(std::string &chan)
{
  _sock->send("JOIN " + chan + "\r\n");
}

void irc::join(std::string &chan, std::string &key)
{
  _sock->send("JOIN " + chan + " " + key + "\r\n");
}

void irc::part(std::string &chan)
{
  _sock->send("PART " + chan + "\r\n");
}

void irc::mode(std::string &target, std::string &mode)
{
  _sock->send("MODE " + target + " " + mode + "\r\n");
}

void irc::mode(std::string &target, std::string &mode, std::string &param)
{
  _sock->send("MODE " + target + " " + mode + " " + param + "\r\n");
}

void irc::kick(std::string &channel, std::string &target, std::string &reason)
{
  _sock->send("KICK " + channel + " " + target + ":" + reason + "\r\n");
}

void irc::notice(std::string target, std::string msg)
{
  _sock->send("NOTICE " + target + " :" + msg + "\r\n");
}

void irc::pong()
{
  std::string server;
  
  server = Utils::getToken(_buf, ":", 2);
  _sock->send("PONG " + server + "\r\n");
}

void irc::identify()
{
  std::string identify;

  if (_nspass == "" || _nickserv == "")
    return;
  identify = "IDENTIFY " + _nspass;
  privmsg(_nickserv, identify);
}

void irc::_oper(std::string &nick, std::string &pass)
{
  if (!_isOper)
    return;
  _sock->send("OPER " + nick + " " + pass + "\r\n");
}

void irc::_oper(std::string &pass)
{
  _oper(_nick, pass);
}

bool irc::oper()
{
  _oper(_opernick, _operpass);
  return _isOper;
}

void irc::raw(std::string raw)
{
  _sock->send(raw + "\r\n");
}

void irc::run()
{
  if ((_buf = _sock->recv()) == "")
    return;
  if (Utils::getToken(_buf, " ", 1) == "PING")
    pong();
  checkActions();
}

static std::vector<std::pair<std::string, ircEvent> > eventab;

ircEvent	irc::getEvent()
{
  std::string	token;
  std::vector<std::pair<std::string, ircEvent> >::const_iterator it;

  if (eventab.empty())
    {
      eventab.push_back(std::make_pair("JOIN", JOIN));
      eventab.push_back(std::make_pair("PART", PART));
      eventab.push_back(std::make_pair("QUIT", QUIT));
      eventab.push_back(std::make_pair("KICK", KICK));
      eventab.push_back(std::make_pair("MODE", MODE));
      eventab.push_back(std::make_pair("NICK", NICK));
      eventab.push_back(std::make_pair("NOTICE", NOTICE));
      eventab.push_back(std::make_pair("PRIVMSG", PRIVMSG));
    }

  token = Utils::getToken(_buf, " ", 2);
  
  for (it = eventab.begin(); it != eventab.end(); ++it)
    if (it->first == token)
      return it->second;
  return UNKNOWN;
}

User		irc::makeUser(std::string &buf)
{
  User		u;
  size_t	pos, pos2;
  
  if ((pos = buf.find("!")) == std::string::npos)
    throw Exception("Unable to get the nickname");

  u.setNick(buf.substr(1, pos - 1));
  
  if ((pos2 = buf.find("@")) == std::string::npos)
    throw Exception("Unable to get the ident");

  u.setIdent(buf.substr(pos + 1, pos2 - (pos + 1)));

  if ((pos = buf.find(" ")) == std::string::npos)
    throw Exception("Unable to get the hostname");

  u.setHost(buf.substr(pos2 + 1, pos - (pos2 + 1)));

  return u;
}
