#include	<iostream>
#include	"activity.hpp"

activityMonitor::~activityMonitor()
{
  if (_events)
    delete _events;
}

activityMonitor::activityMonitor(int const fd)
  : _fd(fd)
{
  _events = new struct epoll_event[MAX_EVENTS];
  _epfd = -1;
  _flags = 0;
}

activityMonitor::activityMonitor()
{
  _events = new struct epoll_event[MAX_EVENTS];
  _epfd = -1;
  _flags = 0;
}

void activityMonitor::setFd(int const fd)
{
  _fd = fd;
}

void activityMonitor::setEvents(uint32_t const flags)
{
  _flags = flags;
}

bool activityMonitor::init()
{
  if ((_epfd = epoll_create(MAX_EVENTS)) < 0)
    return false;

  if (!_flags)
    return false;

  _ev.events = _flags;
  _ev.data.fd = _fd;

  if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _fd, &_ev) < 0)
    return false;

  return true;
}

bool activityMonitor::checkActivity()
{

  if ((_nbActivity = epoll_wait(_epfd, _events, MAX_EVENTS, -1)) < 0)
    return false;

  return true;
}
