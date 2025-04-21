#include "filter.h"
#include "ip-parse.h"
#include "ip.h"
#include "iptool.h"
#include <stdio.h>
#include <stdlib.h>

int cmd_filter_proc(int cmd_opt_c, struct cmd_opt *cmd_opts, struct cmd_in *in,
                    FILE *out) {
  iap_ip_t *r = (void *)0;
  int rc;
  if (in->kind == CMD_IN_FILE) {
    rc = iap_ip_parse_from_file(in->file, &r);
  } else if (in->kind == CMD_IN_INPLACE) {
    rc = iap_ip_parse_from_str_list(in->inplace_len, in->inplace, &r);
  }

  if (rc) {
    if (rc == 1)
      fprintf(stderr, "invalid input: %s", iap_ip_parse_last_error_str());
    else if (rc == 2)
      fprintf(stderr, "memory allocation error.\n");
    return EXIT_FAILURE;
  }

  iap_free(&r);
  return EXIT_SUCCESS;
}