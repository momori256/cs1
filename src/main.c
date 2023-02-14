#include <stdint.h>

#include "db.h"
#include "ep.h"
#include "err.h"
#include "sv.h"

typedef struct sockaddr_in sockaddr_in;
typedef struct epoll_event epoll_event;

typedef struct {
  MYSQL* const conn;
  int listen_fd;
  int epoll_fd;
} Context;

static const char* const PORT = "54321";
static const int LISTEN_BACKLOG = 10;
static const int MAX_EVENTS = 10;

static int parse_command(const char* const req);
static bool process_request(int fd, MYSQL* const conn);
static bool process_event(const epoll_event* const ev, void* const context);
static void event_loop();

static char* query1_process_row(char* head, const MYSQL_ROW* const row) {
  return head + sprintf(head, "%d, %s\n", atoi((*row)[0]), (*row)[1]);
}

static const Query s_query1 = {"select sleep(1), name from tb1",
                               query1_process_row};

static char* query2_process_row(char* head, const MYSQL_ROW* const row) {
  return head + sprintf(head, "%s, %s\n", (*row)[0], (*row)[1]);
}

static const Query s_query2 = {
    "select t1.name, t2.name "
    "from tb1 as t1 "
    "inner join tb1 as t2 on t1.name != t2.name",
    query2_process_row};

static int parse_command(const char* const req) {
  const char* p = strstr(req, "GET /");
  if (p == NULL) {
    return 0;
  }
  const char* const end = strstr(req, " HTTP/");
  if (end == NULL) {
    return 0;
  }

  p += strlen("GET /");
  int cmd = 0;
  while (p != end) {
    cmd *= 10;
    cmd += *p - '0';
    ++p;
  }
  return cmd;
}

/**
 * @brief Process each request.
 */
static bool process_request(int fd, MYSQL* const conn) {
  char buf[256];
  const int nread = read(fd, buf, sizeof(buf));
  if (nread == -1) {
    PRINT_ERR("read");
    exit(1);
  }

  if (strncmp(buf, "exit", strlen("exit")) == 0) {
    return true;
  }

  const int cmd = parse_command(buf);
  char rows[512] = "\0";
  switch (cmd) {
    case 1:
      db_mysql_query_store(conn, &s_query1, rows);
      break;
    case 2:
      db_mysql_query_store(conn, &s_query2, rows);
      break;
    default:
      return true;
  }

  const size_t nrows = strlen(rows);
  char res[1024];
  const ssize_t nres =
      snprintf(res, sizeof(res),
               "HTTP/1.0 200 OK\r\nContent-Length: %lu\r\n\r\n%s", nrows, rows);
  const ssize_t nwrite = write(fd, res, nres);
  if (nwrite == -1) {
    PRINT_ERR("write");
    exit(1);
  }
  return false;
}

/**
 * @brief Process each event.
 */
static bool process_event(const epoll_event* const ev, void* context) {
  Context* const ctx = (Context*)context;

  if (ev->data.fd != ctx->listen_fd) {
    const bool cancel = process_request(ev->data.fd, ctx->conn);
    ep_del_entry(ctx->epoll_fd, ev->data.fd);
    if (close(ev->data.fd) == -1) {
      PRINT_ERR("close");
    }
    return cancel;
  }

  sockaddr_in peer_addr;
  const int pfd = sv_accept(ctx->listen_fd, &peer_addr);
  ep_add_entry(ctx->epoll_fd, pfd, EPOLLIN);
  return false;
}

static void event_loop() {
  MYSQL* const conn =
      db_create_mysql_connect("localhost", "root", "root", "db1", 3306);
  if (!conn) {
    exit(1);
  }

  const int lfd = sv_create_listen_sock(PORT, LISTEN_BACKLOG);
  const int efd = ep_create_fd();
  ep_add_entry(efd, lfd, EPOLLIN);

  Context ctx = {conn, lfd, efd};
  while (1) {
    if (ep_wait(efd, MAX_EVENTS, -1, (void*)&ctx, process_event)) {
      break;
    }
  }
  db_mysql_finalize(conn);
}

int main() {
  db_mysql_initialize();
  event_loop();
  return 0;
}
