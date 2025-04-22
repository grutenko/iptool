#ifndef proc_h
#define proc_h

#include "iap.h"
#include <stdio.h>

int cmd_invert_proc(struct cmd_data *data);
int cmd_filter_proc(struct cmd_data *data);
int cmd_deflate_proc(struct cmd_data *data);
int cmd_inflate_proc(struct cmd_data *data);

#endif