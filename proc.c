#include "iap.h"
#include "ip.h"

#include <stdlib.h>
#include <string.h>

static struct cmd_opt *opt_find(int cmd_opt_i, struct cmd_opt *opt,
                                const char *_short, const char *_long) {
  int i;
  for (i = 0; i < cmd_opt_i; i++) {
    if ((_short && opt[i].key_c == strlen(_short) &&
         strncmp(opt[i].key, _short, opt[i].key_c) == 0) ||
        (_long && opt[i].key_c == strlen(_long) &&
         strncmp(opt[i].key, _long, opt[i].key_c) == 0))
      return &opt[i];
  }
  return (void *)0;
}

static iap_ip_t *parse_input(struct cmd_data *data) {
  iap_ip_t *r = NULL;
  int rc;
  if (data->in->kind == CMD_IN_FILE) {
    rc = iap_ip_parse_from_file(data->in->file, &r);
  } else if (data->in->kind == CMD_IN_INPLACE) {
    rc = iap_ip_parse_from_str_list(data->in->inplace_len, data->in->inplace,
                                    &r);
  }

  if (rc) {
    if (rc == 1)
      fprintf(stderr, "invert: invalid input: %s",
              iap_ip_parse_last_error_str());
    else if (rc == 2)
      fprintf(stderr, "invert: memory allocation error.\n");
    exit(EXIT_FAILURE);
  }

  return r;
}

int cmd_filter_proc(struct cmd_data *data) {
  iap_ip_t *r = NULL;
  r = parse_input(data);

  struct cmd_opt *opt = opt_find(data->cmd_opt_c, data->cmd_opts, NULL, "by");

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
      fwrite("\n", 1, 1, inflate_out);
      iap_increment(&i);
    }
    break;
  case IAP_WALK_POSTORDER:
    break;
  }
}

int cmd_inflate_proc(struct cmd_data *data) {
  iap_ip_t *r = NULL;
  r = parse_input(data);

  inflate_out = data->out;
  iap_walk(r, inflate_walk_proc);

  fflush(data->out);
  iap_free(&r);

  return EXIT_SUCCESS;
}

int cmd_invert_proc(struct cmd_data *data) {
  iap_ip_t *r = NULL;
  r = parse_input(data);

  iap_free(&r);
  return EXIT_SUCCESS;
}

int cmd_deflate_proc(struct cmd_data *data) {
  iap_ip_t *r = NULL;
  r = parse_input(data);

  iap_free(&r);
  return EXIT_SUCCESS;
}