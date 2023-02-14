#ifndef EP_H
#define EP_H

#include <sys/epoll.h>
#include <stdbool.h>

typedef struct epoll_event epoll_event;

/**
 * @brief Create epoll file descriptor.
 * @return Created file descriptor.
 */
int ep_create_fd();

/**
 * @brief Add an entry to epoll interest list.
 */
void ep_add_entry(int epoll_fd, int fd, enum EPOLL_EVENTS event);

/**
 * @brief Delete an entry from epoll interest list.
 */
void ep_del_entry(int epoll_fd, int fd);

/**
 * @brief Wait and process events.
 * @return true if cancel is requested.
 */
bool ep_wait(int epoll_fd, int max_events, int timeout, void* ctx,
             bool (*process)(const epoll_event* const, void* ctx));

#endif
