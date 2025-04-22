#include "iap.h"
#include "ip.h"

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
      fprintf(stderr, "filter: invalid input: %s",
              iap_ip_parse_last_error_str());
    else if (rc == 2)
      fprintf(stderr, "filter: memory allocation error.\n");
    return EXIT_FAILURE;
  }

  struct cmd_opt *opt = opt_find(cmd_opt_c, cmd_opts, NULL, "by");

  if (!opt) {
    fprintf(stderr, "filter: --by is required.\n");
    return EXIT_FAILURE;
  }

  iap_free(&r);
  return EXIT_SUCCESS;
}

int cmd_inflate_proc(int cmd_opt_c, struct cmd_opt *cmd_opts, struct cmd_in *in,
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
      fprintf(stderr, "filter: invalid input: %s",
              iap_ip_parse_last_error_str());
    else if (rc == 2)
      fprintf(stderr, "filter: memory allocation error.\n");
    return EXIT_FAILURE;
  }

  char buffer[IAP_BEST_LEN + 1] = {0};
  iap_ctx_t ctx = {0};
  iap_ip_t *a, *i, *max;
  int len;
  unsigned int raw_max;
  for (a = iap_begin(r, &ctx); a; a = iap_next(&ctx)) {
    iap_net_from(a, i);
    iap_net_to(a, max);
    raw_max = iap_raw(max);

    while (iap_raw(i) <= raw_max) {
      len = iap_ip_to_a(i, buffer);
      fwrite(buffer, len, 1, out);
      putc('\n', out);
      iap_increment(i);
    }
  }

  iap_free(&r);
  fflush(out);

  return EXIT_SUCCESS;
}

int cmd_invert_proc(int cmd_opt_c, struct cmd_opt *cmd_opts, struct cmd_in *in,
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
      fprintf(stderr, "invert: invalid input: %s",
              iap_ip_parse_last_error_str());
    else if (rc == 2)
      fprintf(stderr, "invert: memory allocation error.\n");
    return EXIT_FAILURE;
  }

  iap_ctx_t ctx = {0};
  iap_ip_t *a;
  for (a = iap_begin(r, &ctx); a; a = iap_next(&ctx))
    ;

  iap_free(&r);
  return EXIT_SUCCESS;
}