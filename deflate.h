#ifndef deflate_h
#define deflate_h


#include "iap.h"
#include <stdio.h>

int cmd_deflate_proc(int cmd_opt_c, struct cmd_opt *cmd_opts, struct cmd_in *in, FILE *out);

#endif