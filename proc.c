#include "ip.h"

#include <stdlib.h>
#include <stdio.h>

int cmd_filter_proc(iap_t **r, FILE *out) { return EXIT_SUCCESS; }

FILE *inflate_out;

void inflate_walk_proc(const iap_t *a, int level, int mode) {
  if (mode != IAP_WALK_INORDER)
    return;

  iap_t i = {0}, max = {0};
  char buffer[IAP_BEST_LEN + 1] = {0};
  iap_from(a, &i);
  iap_to(a, &max);
  unsigned int raw_max = iap_raw(&max);

  while (iap_raw(&i) <= raw_max) {
    int len = iap_ntoa(&i, buffer);
    fwrite(buffer, len, 1, inflate_out);
    putc('\n', inflate_out);
    iap_inc(&i);
  }
}

int cmd_inflate_proc(iap_t **r, FILE *out) {
  inflate_out = out;
  iap_walk(*r, inflate_walk_proc);
  fflush(out);
  return EXIT_SUCCESS;
}

int cmd_invert_proc(iap_t **r, FILE *out) { return EXIT_SUCCESS; }

int cmd_deflate_proc(iap_t **r, FILE *out) { return EXIT_SUCCESS; }
