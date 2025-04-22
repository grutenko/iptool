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

FILE *inflate_out;
char inflate_buffer[IAP_BEST_LEN + 1] = {0};

void inflate_walk_proc(iap_ip_t *a, int mode) {
  iap_ip_t i = {0}, max = {0};
  int len;
  unsigned int raw_max;

  switch (mode) {
  case IAP_WALK_PREORDER:
    break;
  case IAP_WALK_INORDER:
    iap_net_from(a, &i);
    iap_net_to(a, &max);
    raw_max = iap_raw(&max);

    while (iap_raw(&i) <= raw_max) {
      len = iap_ip_to_a(&i, inflate_buffer);
      fwrite(inflate_buffer, len, 1, inflate_out);
      fwrite(", ", 2, 1, inflate_out);
      iap_increment(&i);
    }
    break;
  case IAP_WALK_POSTORDER:
    break;
  }
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
      fprintf(stderr, "inflate: invalid input: %s",
              iap_ip_parse_last_error_str());
    else if (rc == 2)
      fprintf(stderr, "inflate: memory allocation error.\n");
    return EXIT_FAILURE;
  }

  inflate_out = out;
  iap_walk(r, inflate_walk_proc);

  fflush(out);
  iap_free(&r);

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

  iap_free(&r);
  return EXIT_SUCCESS;
}

void cmd_deflate_proc(int cmd_opt_c, struct cmd_opt *cmd_opts,
                      struct cmd_in *in, FILE *out) {
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

  iap_free(&r);
  return EXIT_SUCCESS;
}