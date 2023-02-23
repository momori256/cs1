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
 * @brief Finialize MySQL.
 * @param conn MySQL connection.
 */
void db_mysql_finalize();

/**
 * @brief Create MySQL database connection.
 * @return Created connection if succeeded; otherwise NULL.
 */
MYSQL* db_create_mysql_connect(const char* const host, const char* const user,
                               const char* const passwd, const char* const db,
                               unsigned int port);

/**
 * @brief Finialize MySQL database connection.
 */
void db_finalize_mysql_connect(MYSQL* conn);

/**
 * @brief Execute MySQL query.
 * @param conn MySQL connection.
 * @param query Query to execute.
 * @param buf Buffer to store results.
 */
void db_mysql_query_store(MYSQL* const conn, const Query* const query,
                          char* const buf);
