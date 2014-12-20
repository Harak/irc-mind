#ifndef __FLUXBB_HPP__
#define __FLUXBB_HPP__

#include <string>
#include <vector>
#include "module.hpp"

struct newpost
{
  std::string forum;
  std::string poster;
  std::string subject;
  std::string id;
  std::string posted;
};

enum requestType
  {
    newPost = 0,
    newTopic
  };

class FluxBB : public Module
{
  std::vector<newpost*> _post;
  std::vector<newpost*> _topic;
  std::string		_users;

  std::string	_lastpost;
  std::string	_lasttopic;
  std::string	_lastuser;

  std::string	_posturl;
  std::string	_topicurl;
  /*  std::string	_userurl; */

  std::string	_channel;
  std::string	_baseUrl;
  std::string	_key;

public:

  FluxBB();
  virtual ~FluxBB();

  void init();
  void run();

private:
  
  void checkNewTopic();
  void checkNewPost();
  /* void checkNewUser(); */

  void getDataUsers();
  void getData(requestType);
  void loadXml(requestType);
};

#endif
