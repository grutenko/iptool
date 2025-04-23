#include "cmd.h"
#include "core.h"

#include <stdio.h>

void walk(const iap_t *a, int depth, int mode, void *data) {
  if (mode != IAP_WALK_INORDER)
    return;

  iap_t from = {0}, to = {0};
  iap_from(a, &from);
  iap_to(a, &to);
  char buffer[IAP_BEST_LEN + 1] = {0};

  while (iap_raw(&from) <= iap_raw(&to)) {
    iap_ntoa((const iap_t *)&from, buffer);
    printf("%s\n", buffer);
    iap_inc(&from);
  }
}

int cmd_inflate(int argc, char **argv) {
  // Implementation of the deflate command
  iap_t *root = (void *)0;
  parse_ips(argc, argv, &root);
  iap_walk(root, walk, (void *)0);
  iap_free(&root);
  return 0;
}

void cmd_inflate_help() {}
