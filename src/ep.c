#include "ep.h"

#include "err.h"

int ep_create_fd() {
  const int efd = epoll_create1(0);
  if (efd == -1) {
    PRINT_ERR("epoll_create");
  }
  return efd;
}

void ep_add_entry(int epoll_fd, int fd, enum EPOLL_EVENTS event) {
  epoll_event ev;
  ev.events = event;
  ev.data.fd = fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    PRINT_ERR("epoll_ctl");
  }
}

void ep_del_entry(int epoll_fd, int fd) {
  if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
    PRINT_ERR("epoll_ctl");
  }
}

bool ep_wait(int epoll_fd, int max_events, int timeout, void* ctx,
             bool (*process)(const epoll_event* const, void*)) {
  epoll_event events[max_events];
  const int nfds = epoll_wait(epoll_fd, events, max_events, timeout);
  if (nfds == -1) {
    PRINT_ERR("epoll_wait");
  }

  bool cancel = false;
  for (int i = 0; i < nfds; ++i) {
    cancel |= process(&events[i], ctx);
  }
  return cancel;
}
