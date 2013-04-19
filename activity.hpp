#ifndef __ACTIVITY_MONITOR__
#define __ACTIVITY_MONITOR__

#include <sys/types.h>
#include <sys/epoll.h>

#define	MAX_EVENTS 10

class activityMonitor
{
  int			_fd;
  int			_epfd;
  struct epoll_event	_ev;
  struct epoll_event	*_events;
  uint32_t		_flags;
  int			_nbActivity;

public:
  activityMonitor();
  activityMonitor(int const);
  ~activityMonitor();

  void setFd(int const);
  void setEvents(uint32_t const);

  bool init();

  bool checkActivity();

  int  getNb() const { return _nbActivity; }
  struct epoll_event *getActivity() const { return _events; }
};

#endif
