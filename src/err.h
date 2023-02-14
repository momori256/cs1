#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRINT_ERR(msg)                               \
  fprintf(stderr, "%s: %s\n", msg, strerror(errno)); \
  exit(1);
