#ifndef __SAMPLE__HPP__
#define __SAMPLE__HPP__

#include "module.hpp"

class Sample : public Module
{
private:
  std::string _sampleChannel;

public:
  Sample();
  virtual ~Sample();

  // You have to implement those
  void init();
  void run();

  // Optionnal event actions
  void onPrivmsg(User &, std::string);
  void onPart(User &, std::string, std::string);
};

#endif
