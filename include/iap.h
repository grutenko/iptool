#ifndef iap_h
#define iap_h

#include <stdio.h>

#define FAILURE(...)                                                           \
  do {                                                                         \
    fprintf(stderr, __VA_ARGS__);                                              \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define SHORT_USAGE                                                            \
  "iap <command> [options] [args]. Use iap help for more information.\n"

#endif
