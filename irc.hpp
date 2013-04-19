#ifndef		__IRC_HPP__
#define		__IRC_HPP__

#include	<list>
#include	<utility>
#include	<vector>
#include	"user.hpp"
#include	"socket.hpp"

#define		DEFAULT_QUIT_MSG "bye."

enum ircEvent
  {
    JOIN,
    PART,
    QUIT,
    KICK,
    MODE,
    NOTICE,
    NICK,
    PRIVMSG,
    UNKNOWN
  };

class irc;

typedef		std::pair<std::string, bool (irc::*)()> ircExec;

class irc
{
  int		_port;
  std::string	_server;
  std::string	_nick;
  std::string	_anick;
  std::string	_ident;
  std::string	_realname;
  std::string	_nickserv;
  std::string	_opernick;
  std::string	_nspass;
  std::string	_operpass;

  clientSocket	*_sock;
  std::string	_buf;

  bool		_isOper;
  std::vector<ircExec> action;

  bool		checkActions();
  bool		anick();

public:

  irc();
  irc(const irc &);
  ~irc();

  bool		connect(std::string, int);
  bool		connect(std::string);
  bool		connect(char const *, int);
  bool		connect(char const *);
  bool		auth();
  void		run();
  void		end();
  void		setNick(std::string const);
  void		setNick(char const *);
  void		setIdent(std::string const);
  void		setIdent(char const *);
  void		setRealname(std::string const);
  void		setRealname(char const *);
  void		setServer(std::string const);
  void		setServer(char const *);
  clientSocket  *getSock() const;
  int		getFd() const;
  std::string	getBuf() const { return _buf; }
  void		setNsPass(std::string const);
  void		setNsPass(char const *);
  void		setNsNick(std::string const);
  void		setNsNick(char const *);
  void		setOper(bool);
  void		setOperNick(std::string const);
  void		setOperNick(char const *);
  void		setOperPass(std::string const);
  void		setOperPass(char const *);
  static User  	makeUser(std::string &);
  ircEvent	getEvent();
  bool		isOper();

  // IRC methods
  void		pong();
  void		user();
  void		user(std::string &, std::string &, std::string &);
  void		nick(std::string &);
  void		nick();
  void		quit(std::string &);
  void		privmsg(std::string, std::string);
  void		join(std::string &);
  void		join(std::string &, std::string &);
  void		part(std::string &);
  void		mode(std::string &, std::string &);
  void		mode(std::string &, std::string &, std::string &);
  void		kick(std::string &, std::string &, std::string &);
  void		notice(std::string, std::string);
  void		identify();
  void		_oper(std::string &, std::string &);
  void		_oper(std::string &);
  bool		oper();
  void		raw(std::string);
};

#endif
