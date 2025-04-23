#include "cmd.h"
#include "core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clang-format off

struct cmd_struct commands[] = {
    {"invert", "Invert list of subnets.", cmd_invert_proc, cmd_invert_help_proc},
    {"filter", "filter list of addresses by subnet.", cmd_filter_proc, cmd_filter_help_proc},
    {"inflate", "inflate (expand list of subnets)", cmd_inflate_proc, cmd_inflate_help_proc},
    {"deflate", "deflate (find subnets)", cmd_deflate_proc, cmd_deflate_help_proc},
    {"help", "Show help message", cmd_help_proc, NULL},
    {"list", "List all commands", cmd_list_proc, NULL},
    {0, 0}};

// clang-format on

struct cmd_struct *find_cmd(const char *name) {
  for (size_t i = 0; commands[i].name; i++) {
    if (strcmp(commands[i].name, name) == 0)
      return &commands[i];
  }
  return NULL;
}

void cmd_filter_help_proc() {}

int cmd_filter_proc(int argc, char **argv) { return EXIT_SUCCESS; }

void inflate_walk_proc(const iap_t *a, int level, int mode, void *data) {
  if (mode != IAP_WALK_INORDER)
    return;

  iap_t i = {0}, max = {0};
  char buffer[IAP_BEST_LEN + 1] = {0};
  iap_from(a, &i);
  iap_to(a, &max);
  unsigned int raw_max = iap_raw(&max);

  while (iap_raw(&i) <= raw_max) {
    int len = iap_ntoa(&i, buffer);
    fwrite(buffer, len, 1, (FILE *)data);
    putc('\n', (FILE *)data);
    iap_inc(&i);
  }
}

void cmd_inflate_help_proc() {}

int cmd_inflate_proc(int argc, char **argv) {
  // iap_walk(*r, inflate_walk_proc, (void *)out);
  return EXIT_SUCCESS;
}

void cmd_invert_help_proc() {}

int cmd_invert_proc(int argc, char **argv) { return EXIT_SUCCESS; }

void cmd_deflate_help_proc() {}

int cmd_deflate_proc(int argc, char **argv) { return EXIT_SUCCESS; }

int cmd_help_proc(int argc, char **argv) { return EXIT_SUCCESS; }

int cmd_list_proc(int argc, char **argv) { return EXIT_SUCCESS; }
