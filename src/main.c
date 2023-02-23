#include <pthread.h>
#include <stdint.h>

#include "db.h"
#include "ep.h"
#include "err.h"
#include "sv.h"

typedef struct sockaddr_in sockaddr_in;
typedef struct epoll_event epoll_event;

typedef struct {
  int listen_fd;
  int epoll_fd;
} Context;

typedef struct {
  int epoll_fd;
  int client_fd;
  char* req;
} ThContext;

static const char* const PORT = "54321";
static const int LISTEN_BACKLOG = 10;
static const int MAX_EVENTS = 10;
static const int REQ_SIZE = 256;
static pthread_mutex_t canceled_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool canceled = false;

static int parse_command(const char* const req);
static bool process_request(int fd, const char* const req);
static void* process_request_thread(void* arg);
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

  int cmd = 0;
  p += strlen("GET /");
  while ('0' <= *p && *p <= '9') {
    cmd *= 10;
    cmd += *p - '0';
    ++p;
  }
  return cmd;
}

/**
 * @brief Process each request.
 */
static bool process_request(int fd, const char* const req) {
  if (strncmp(req, "exit", strlen("exit")) == 0) {
    return true;
  }

  MYSQL* const conn =
      db_create_mysql_connect("localhost", "root", "root", "db1", 3306);
  if (!conn) {
    exit(1);
  }

  const int cmd = parse_command(req);
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

  db_finalize_mysql_connect(conn);

  const size_t nrows = strlen(rows);
  char res[1024];
  const ssize_t nres =
      snprintf(res, sizeof(res),
               "HTTP/1.0 200 OK\r\nContent-Length: %lu\r\n\r\n%s", nrows, rows);
  const ssize_t nwrite = write(fd, res, nres);
  if (nwrite == -1) {
    PRINT_ERR("write");
  }
  if (close(fd) == -1) {
    PRINT_ERR("close");
  }
  return false;
}

static void* process_request_thread(void* arg) {
#ifdef MULTI_THREAD
  if (pthread_detach(pthread_self())) {
    PRINT_ERR("pthread_detach");
  }
#endif
  ThContext* const ctx = (ThContext*)arg;
  const int fd = ctx->client_fd;

#ifdef MULTI_THREAD
  // printf("thread begin. id[%lu], fd[%d]\n", pthread_self(), fd);
#endif
  const bool cancel = process_request(fd, ctx->req);
  free(ctx->req);
  free(ctx);

  {
#ifdef MULTI_THREAD
    if (pthread_mutex_lock(&canceled_mutex)) {
      PRINT_ERR("pthread_mutex_lock");
    }
#endif
    canceled |= cancel;
#ifdef MULTI_THREAD
    if (pthread_mutex_unlock(&canceled_mutex)) {
      PRINT_ERR("pthread_mutex_unlock");
    }
#endif
  }
  return NULL;
}

/**
 * @brief Process each event.
 */
static bool process_event(const epoll_event* const ev, void* context) {
  Context* const ctx = (Context*)context;

  if (ev->data.fd != ctx->listen_fd) {
    ep_del_entry(ctx->epoll_fd, ev->data.fd);

    char* const req = (char*)malloc(sizeof(char) * REQ_SIZE);
    const int nread = read(ev->data.fd, req, REQ_SIZE);
    if (nread == -1) {
      PRINT_ERR("read");
      exit(1);
    }

    ThContext* th_ctx = (ThContext*)malloc(sizeof(ThContext));
    {
      th_ctx->epoll_fd = ctx->epoll_fd;
      th_ctx->client_fd = ev->data.fd;
      th_ctx->req = req;
    }
#ifdef MULTI_THREAD
    pthread_t tid = 0;
    if (pthread_create(&tid, NULL, process_request_thread, (void*)th_ctx)) {
      PRINT_ERR("pthread_create");
    }
#else
    process_request_thread((void*)th_ctx);
#endif
    return false;
  }

  sockaddr_in peer_addr;
  const int pfd = sv_accept(ctx->listen_fd, &peer_addr);
  ep_add_entry(ctx->epoll_fd, pfd, EPOLLIN);
  return false;
}

static void event_loop() {
  const int lfd = sv_create_listen_sock(PORT, LISTEN_BACKLOG);
  const int efd = ep_create_fd();
  ep_add_entry(efd, lfd, EPOLLIN);

  Context ctx = {lfd, efd};
  while (1) {
    if (ep_wait(efd, MAX_EVENTS, -1, (void*)&ctx, process_event)) {
      break;
    }
  }
}

int main() {
  db_mysql_initialize();
  event_loop();
  db_mysql_finalize();
  if (pthread_mutex_destroy(&canceled_mutex)) {
    PRINT_ERR("pthread_mutex_destroy");
  }
  return 0;
}
