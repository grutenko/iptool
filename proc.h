#ifndef proc_h
#define proc_h

#include "ip.h"
#include <stdio.h>

int cmd_invert_proc(iap_t **r, FILE *out);
int cmd_filter_proc(iap_t **r, FILE *out);
int cmd_deflate_proc(iap_t **r, FILE *out);
int cmd_inflate_proc(iap_t **r, FILE *out);

#endif
