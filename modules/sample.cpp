#include <unistd.h> // sleep(1)
#include "sample.hpp"

Sample::Sample()
  : Module("sample")
{
}

Sample::~Sample()
{

}

void Sample::init()
{
  // this is called just after the module symbols are loaded
  _threaded = true; // creates a new thread for the run() method. Default: false
  _event = true;
}

void Sample::run()
{
  // called after init (in anoter thread if _threaded was set to 'true' in the Constructor or the init() method
  // do whatever you want
  int i = 0;
  _sampleChannel = "#channel";

  _irc->join(_sampleChannel);

  while (1) // will block the rest of the bot executio if _threaded is not true
    {
      _irc->privmsg(_sampleChannel, Utils::intToString(i++));
      sleep(1); // will sleep the whole bot if _threaded is not true
    }
}

//
// Events
//
void Sample::onPrivmsg(User &u, std::string msg)
{
  *_logger << u.getNick() + " sent a msg: " + msg;
  _irc->privmsg(u.getNick(), "Hello there!");
}

void Sample::onPart(User &u, std::string target, std::string msg)
{
  _irc->privmsg("#channel", u.getNick() + " left the channel " + target + "(" + msg + ")");
}


/* Do not edit the following code */
extern "C"
{
  Module	*create()
  {
    return new Sample;
  }
  void		destroy(Module *m)
  {
    delete m;
  }
}

