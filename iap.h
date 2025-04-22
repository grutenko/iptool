#ifndef iap_h
#define iap_h

#include <stdio.h>

struct cmd_opt {
  int key_c;
  int value_c;
  char *key;
  char *value;
};

enum {
  CMD_IN_INPLACE = 0,
  CMD_IN_FILE = 1
};

struct cmd_in {
  int kind;
  int inplace_len;
  const char **inplace;
  FILE *file;
};

typedef int (*cmd_proc_p)(int cmd_opt_c, struct cmd_opt *cmd_opts, struct cmd_in *in, FILE *out);

struct cmd_opt *opt_find(int cmd_opt_i, struct cmd_opt *opt,
  const char *_short, const char *_long);

#endif