#include "db.h"

#include "err.h"

void db_mysql_initialize() {
  if (mysql_library_init(0, NULL, NULL)) {
    PRINT_ERR("mysql_library_init");
  }
}

MYSQL* db_create_mysql_connect(const char* const host, const char* const user,
                               const char* const passwd, const char* const db,
                               unsigned int port) {
  MYSQL* const conn = mysql_init(NULL);
  if (!conn) {
    PRINT_ERR("mysql_init");
  }

  if (!mysql_real_connect(conn, host, user, passwd, db, port, NULL, 0)) {
    PRINT_ERR("mysql_real_connect");
  }

  return conn;
}

void db_mysql_finalize() {
  mysql_library_end();
}

void db_finalize_mysql_connect(MYSQL* conn) {
  mysql_close(conn);
}

void db_mysql_query_store(MYSQL* const conn, const Query* const query,
                          char* const buf) {
  if (mysql_query(conn, query->sql)) {
    PRINT_ERR("mysql_query");
    mysql_close(conn);
  }

  MYSQL_RES* const res = mysql_store_result(conn);
  MYSQL_ROW row;

  char* p = buf;
  while ((row = mysql_fetch_row(res)) != NULL) {
    p = query->process_row(p, &row);
  }
  mysql_free_result(res);
}
