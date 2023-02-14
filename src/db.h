#pragma once

#include <mysql/mysql.h>
#include <stdbool.h>

typedef struct {
  const char* const sql;
  char* (*process_row)(char* head, const MYSQL_ROW* row);
} Query;

/**
 * @brief Initialize MySQL.
 */
void db_mysql_initialize();

/**
 * @brief Create MySQL database connection.
 * @return Created connection if succeeded; otherwise NULL.
 */
MYSQL* db_create_mysql_connect(const char* const host, const char* const user,
                               const char* const passwd, const char* const db,
                               unsigned int port);

/**
 * @brief Function pointer processing each MYSQL_ROW.
 */
typedef void (*row_processer)(const MYSQL_ROW* const, void* context);

/**
 * @brief Execute MySQL query.
 * @param conn MySQL connection.
 * @param sql SQL to executed.
 * @param process Fuction processing each row of the result.
 */
void db_mysql_query(MYSQL* const conn, const char* const sql, void* context,
                    row_processer process);

void db_mysql_query_store(MYSQL* const conn, const Query* const query,
                          char* const buf);

/**
 * @brief Finialize MySQL.
 * @param conn MySQL connection.
 */
void db_mysql_finalize(MYSQL* conn);
