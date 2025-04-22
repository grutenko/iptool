#ifndef proc_h
#define proc_h

#include "iap.h"
#include <stdio.h>

int cmd_invert_proc(int cmd_opt_c, struct cmd_opt *cmd_opts, struct cmd_in *in,
                    FILE *out);
int cmd_filter_proc(int cmd_opt_c, struct cmd_opt *cmd_opts, struct cmd_in *in,
                    FILE *out);
int cmd_deflate_proc(int cmd_opt_c, struct cmd_opt *cmd_opts, struct cmd_in *in,
                     FILE *out);
int cmd_inflate_proc(int cmd_opt_c, struct cmd_opt *cmd_opts, struct cmd_in *in,
                     FILE *out);

#endif