#include "sv.h"

#include "err.h"

typedef struct addrinfo addrinfo;

int sv_create_listen_sock(const char* const port, int backlog) {
  addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  addrinfo* head;
  {
    const int result = getaddrinfo(NULL, port, &hints, &head);
    if (result != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
      exit(1);
    }
  }

  bool succeeded = false;
  int sfd = 0;

  for (addrinfo* p = head; p != NULL; p = p->ai_next) {
    sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sfd == -1) {
      continue;
    }

    int val = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
      PRINT_ERR("setsockopt");
    }

    if (bind(sfd, p->ai_addr, p->ai_addrlen) != 0) {
      PRINT_ERR("bind");
      if (close(sfd) == -1) {
        PRINT_ERR("close");
      }
      continue;
    }
    succeeded = true;
    break;
  }

  freeaddrinfo(head);
  if (!succeeded) {
    PRINT_ERR("create_socket");
  }

  if (listen(sfd, backlog) == -1) {
    PRINT_ERR("listen");
  }

  return sfd;
}

int sv_accept(int listen_fd, sockaddr_in* const peer) {
  socklen_t peer_len = sizeof(sockaddr_in);
  const int peer_fd = accept(listen_fd, (struct sockaddr*)&peer, &peer_len);
  if (peer_fd == -1) {
    PRINT_ERR("accept");
  }
  return peer_fd;
}
