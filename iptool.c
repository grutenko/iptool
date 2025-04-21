#include "iptool.h"
#include "invert.h"
#include "ip.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct command {
  const char *name;
  cmd_proc_p proc;
} commands[] = {{"invert", cmd_invert_proc}, {0, 0}};

static void help(FILE *o) {
  printf("Usage:\n");
  printf("  iptool <command> [options] [args]\n\n");

  printf("Commands:\n");
  printf("  invert           Invert given subnet list (i.e., find address "
         "ranges not covered)\n");
  printf("  merge            Merge overlapping and adjacent subnets\n");
  printf("  uniq             Remove duplicate IP addresses or subnets\n");
  printf("  filter           Filter addresses by subnet\n");
  printf("  sort             Sort IP addresses or networks\n\n");

  printf("Arguments:\n");
  printf("  [args]           List of IPs/subnets, file path, or '-' for "
         "stdin\n\n");

  printf("Options:\n");
  printf("  --by <subnet>    Subnet used for filtering (only with 'filter')\n");
  printf("  --help           Show this help message\n\n");

  printf("Examples:\n");
  printf("  iptool invert iplist.txt\n");
  printf("  iptool filter --by 192.168.0.0/16 - < input.txt\n");
  printf("  iptool merge 1.1.1.0/24 1.1.2.0/24\n\n");

  printf("Note:\n");
  printf("  All addresses must be in CIDR notation (e.g., 192.168.1.0/24).\n");
}

static void help_exit(int code, FILE *o) {
  help(o);
  exit(code);
}

static void help_exit_fmt(int code, FILE *o, const char *format, ...) {}

static void error(int code, const char *format, ...) {}

static int cmd_parse_option(int argc, const char **argv, struct cmd_opt *opt) {}

static struct cmd_opt *opt_find(int cmd_opt_i, struct cmd_opt *opt) {}

int main(int argc, const char **argv) {
  if (argc == 1)
    help_exit(EXIT_SUCCESS, stdout);

  int i;
  struct cmd_opt cmd_opts[32];
  int cmd_opt_c = 0;

  for (i = 2; i < argc; i++) {
    if (!cmd_parse_option(argc - i, &(argv[i]), &cmd_opts[cmd_opt_c]))
      help_exit_fmt(EXIT_FAILURE, stderr, "Invalid option %s.\n", argv[i]);

    cmd_opt_c++;
  }

  FILE *out = stdout;
  struct cmd_opt *output = opt_find(cmd_opt_c, cmd_opts);

  if (output) {
    char buf[256];
    strncpy(buf, output->value, output->value_c);

    out = fopen(buf, "w");
    if (!out)
      error(EXIT_FAILURE, "output file error: %s\n", strerror(errno));
  }

  struct command *cmd;
  for (i = 0; commands[i].name; i++) {
    if (strcmp(commands[i].name, argv[1]) == 0) {
      cmd = &commands[i];
    }
  }



  return EXIT_SUCCESS;
}