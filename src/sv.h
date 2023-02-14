#ifndef SV_H
#define SV_H

#include <errno.h>    // errno.
#include <netdb.h>    // getaddrinfo, freeaddrinfo, gai_strerror.
#include <stdbool.h>  // bool.
#include <stdint.h>
#include <stdio.h>   // printf, fprintf.
#include <stdlib.h>  // exit.
#include <string.h>  // strerror.
#include <sys/socket.h>  // getaddrinfo, freeaddrinfo, gai_strerror, socket, setsockopt, bind.
#include <sys/types.h>  // getaddrinfo, freeaddrinfo, gai_strerror, socket, setsockopt, bind.
#include <unistd.h>  // read, close.

typedef struct sockaddr_in sockaddr_in;

/**
 * @brief Create a socket for listening.
 * @return Created file descriptor.
 */
int sv_create_listen_sock(const char* const port, int backlog);

/**
 * @brief
 *
 */
int sv_accept();

#endif
