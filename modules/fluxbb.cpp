#include <sstream>
#include <curl/curl.h>
#include <libxml/xmlreader.h>
#include <unistd.h>
#include <time.h>
#include "utils.hpp"
#include "fluxbb.hpp"
#include "moduleexception.hpp"

FluxBB::FluxBB()
  : Module("fluxbb")
{
  _lastpost = Utils::intToString(time(NULL));
  _lasttopic = Utils::intToString(time(NULL));
}

FluxBB::~FluxBB()
{  
}

void		FluxBB::init()
{
  _threaded = true;
  _event = false;

  if (!_conf.exists("FluxBBChannel"))
    throw ModuleException("Missing configuration parameter 'FluxBBChannel'");
  _channel = _conf.get("FluxBBChannel");

  if (!_conf.exists("FluxBBUrl"))
    throw ModuleException("Missing configuration parameter 'FluxBBUrl'");
  _baseUrl = _conf.get("FluxBBUrl");

  if (!_conf.exists("FluxBBKey"))
    throw ModuleException("Missing configuration parameter 'FluxBBKey'");
  _key = _conf.get("FluxBBKey");

  _irc->join(_channel);

}

void		FluxBB::run()
{
  while (1)
    {
      sleep(10);
      checkNewTopic();
      checkNewPost();
      /* checkNewUser(); not ready yet */
    }
}

static size_t		my_read(void *ptr, size_t sz, size_t nb, void *stream)
{
  return fwrite(ptr, sz, nb, (FILE*)stream);
}

void		FluxBB::getData(requestType type)
{
  CURL		*curl_handle;
  FILE		*fd;

  if (!(fd = fopen(type == newPost ? "/tmp/newpost.xml" : "/tmp/newtopic.xml", "w")))
    return;

  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();

  curl_easy_setopt(curl_handle, CURLOPT_URL, (type == newPost ? _posturl : _topicurl).c_str());
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &my_read);
  curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fd);
  curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 3);
  curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5);
  curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 1800);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "mind/0.1");
  curl_easy_perform(curl_handle);
  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();
  fclose(fd);
}

void		FluxBB::checkNewTopic()
{ 
  std::vector<newpost *>::iterator it;

  _topicurl = _baseUrl;
  _topicurl += "?cmd=newtopic&key=";
  _topicurl += Utils::md5(Utils::md5(_key) + "newtopic");
  _topicurl += "&last=" + _lasttopic;

  getData(newTopic);
  loadXml(newTopic);
  for (it = _topic.begin(); it != _topic.end(); ++it)
    {
      newpost *n = *it;
      if (n->forum != "Equipe")
	{
	_irc->privmsg(_channel, "[" + n->forum + "] Nouveau sujet créé par " + n->poster + " : " + n->subject + " " + _baseUrl + "/viewtopic.php?id=" + n->id + "&action=new");
	if (Utils::stringToInt(_lasttopic) < Utils::stringToInt(n->posted))
	  _lasttopic = std::string(n->posted);
	}
      delete n;
    }
  _topic.clear();
}

void		FluxBB::checkNewPost()
{
  std::vector<newpost *>::iterator it;

  _posturl = _baseUrl;
  _posturl += "?cmd=newpost&key=";
  _posturl += Utils::md5(Utils::md5(_key) + "newpost");
  _posturl += "&last=" + _lastpost;

  getData(newPost);
  loadXml(newPost);
  for (it = _post.begin(); it != _post.end(); ++it)
    {
      newpost *n = *it;
      if (n->forum != "Equipe")
	{
	  _irc->privmsg(_channel, "[" + n->forum + "] Nouveau message posté par " + n->poster + " : " + n->subject  + " " + _baseUrl + "/viewtopic.php?id=" + n->id + "&action=new");
	  if (Utils::stringToInt(_lastpost) < Utils::stringToInt(n->posted))
	    _lastpost = std::string(n->posted);
	}
      delete n;
    }
  _post.clear();
}

void		FluxBB::loadXml(requestType type)
{
  std::string	file;
  std::string	t;
  xmlDocPtr	doc;
  xmlNodePtr	node, elem, entry;
  xmlChar	*value;
  newpost	*n;

  t = std::string((type == newPost ? "post" : "topic"));
  file = "/tmp/new" + t + ".xml";
  if (!(doc = xmlParseFile(file.c_str())))
    return;
  if (!(node = xmlDocGetRootElement(doc)))
    {
      xmlFreeDoc(doc);
      return;
    }
  for (elem = node->xmlChildrenNode; elem; elem = elem->next)
    {
      if (std::string((const char *)elem->name) == t)
	{
	  n = new newpost();
	  for (entry = elem->xmlChildrenNode; entry; entry = entry->next)
	    {
	      if (std::string((const char *)entry->name) == "forum")
		{
		  value = xmlNodeListGetString(doc, entry->xmlChildrenNode, 1);
		  if (value)
		    {
		      n->forum = std::string((const char*)value);
		      xmlFree(value);
		    }
		}
	      else if (std::string((const char *)entry->name) == "poster")
		{
		  value = xmlNodeListGetString(doc, entry->xmlChildrenNode, 1);
		  if (value)
		    {
		      n->poster = std::string((const char*)value);
		      xmlFree(value);
		    }
		}
	      else if (std::string((const char *)entry->name) == "id")
		{
		  value = xmlNodeListGetString(doc, entry->xmlChildrenNode, 1);
		  if (value)
		    {
		      n->id = std::string((const char*)value);
		      xmlFree(value);
		    }
		}
	      else if (std::string((const char *)entry->name) == "subject")
		{
		  value = xmlNodeListGetString(doc, entry->xmlChildrenNode, 1);
		  if (value)
		    {
		      n->subject = std::string((const char *)value);
		      xmlFree(value);
		    }
		}
	      else if (std::string((const char *)entry->name) == "posted")
		{
		  value = xmlNodeListGetString(doc, entry->xmlChildrenNode, 1);
		  if (value)
		    {
		      n->posted = std::string((const char *)value);
		      xmlFree(value);
		    }
		}
	    }
	  if (n)
	    {
	      if (type == newPost)
		_post.push_back(n);
	      else
		_topic.push_back(n);
	    }
	}
    }
  xmlFreeDoc(doc);
}

/*
static size_t	get_users(void *ptr, size_t sz, size_t nb, void *userdata)
{
  std::ostringstream *stream = (std::ostringstream*)userdata;
  size_t	     count = sz * nb;

  stream->write((char *)ptr, count);

  return count;
}


void		FluxBB::getDataUsers()
{
  std::string	result;
  CURL		*curl_handle;
  std::ostringstream stream;
  
  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();

  curl_easy_setopt(curl_handle, CURLOPT_URL, _userurl.c_str());
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &get_users);
  curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &stream);
  curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5);
  curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 3);
  curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 1800);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "mind/0.1");
  curl_easy_perform(curl_handle);
  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();

  _users = stream.str();
}


void		FluxBB::checkNewUser()
{
  int		i;
  std::string	tok = "-";

  _userurl = _baseUrl;
  _userurl += "?cmd=newuser&key=";
  _userurl += Utils::md5(Utils::md5(_key) + "newuser");
  _userurl += "&last=" + _lastuser;

  std::cout << _userurl << std::endl;

  getDataUsers();
  if (_users != "")
    {
      if (_users.find("[") == std::string::npos)
	_irc->privmsg(_channel, "[Forum] - Bienvenue au nouvel inscrit : " + _users);
      else
	for (i = 1; tok != ""; ++i)
	  {
	    tok = Utils::getToken(_users, "[", i);
	    if (tok == "")
	      break;
	    _irc->privmsg(_channel, "[Forum] - Bienvenue au nouvel inscrit : " + tok);
	  }
    }
}
*/
extern "C"
{
  Module	*create()
  {
    return new FluxBB;
  }
  void		destroy(Module *m)
  {
    delete m;
  }
}
