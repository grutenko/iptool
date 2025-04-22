#ifndef iap_h
#define iap_h

#include "ip.h"
#include <stdio.h>

struct cmd_opt {
  int key_c;
  int value_c;
  char *key;
  char *value;
};

typedef int (*cmd_proc_p)(iap_t **r, FILE *out);

#endif
