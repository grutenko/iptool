#include "iap.h"
#include "ip.h"
#include "proc.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct command {
  const char *name;
  const char *descr;
  cmd_proc_p proc;
} commands[] = {
    {"invert", "Invert list of subnets.", cmd_invert_proc},
    {"filter", "filter list of addresses by subnet.", cmd_filter_proc},
    {"inflate", "inflate (expand list of subnets)", cmd_inflate_proc},
    {"deflate", "deflate (find subnets)", cmd_deflate_proc},
    {0, 0}};

static void help(FILE *o) {
  fprintf(o, "Usage:\n");
  fprintf(o, "  iptool <command> [options] [args]\n\n");

  fprintf(o, "Commands:\n");
  fprintf(o, "  invert           Invert given subnet list (i.e., find address "
             "ranges not covered)\n");
  fprintf(o, "  merge            Merge overlapping and adjacent subnets\n");
  fprintf(o, "  uniq             Remove duplicate IP addresses or subnets\n");
  fprintf(o, "  filter           Filter addresses by subnet\n");
  fprintf(o, "  sort             Sort IP addresses or networks\n\n");

  fprintf(o, "Arguments:\n");
  fprintf(o, "  [args]           List of IPs/subnets, file path, or '-' for "
             "stdin\n\n");

  fprintf(o, "Options:\n");
  fprintf(
      o, "  --by <subnet>    Subnet used for filtering (only with 'filter')\n");
  fprintf(o, "  --help           Show this help message\n\n");

  fprintf(o, "Examples:\n");
  fprintf(o, "  iptool invert iplist.txt\n");
  fprintf(o, "  iptool filter --by 192.168.0.0/16 - < input.txt\n");
  fprintf(o, "  iptool merge 1.1.1.0/24 1.1.2.0/24\n\n");

  fprintf(o, "Note:\n");
  fprintf(o,
          "  All addresses must be in CIDR notation (e.g., 192.168.1.0/24).\n");
}

static void help_exit(int code, FILE *o) {
  help(o);
  fflush(o);
  exit(code);
}

static void help_exit_fmt(int code, FILE *o, const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  vfprintf(o, format, arg);
  putc('\n', o);
  putc('\n', o);
  va_end(arg);
  help_exit(code, o);
}

static void error(int code, const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  vfprintf(stderr, format, arg);
  va_end(arg);
  exit(code);
}

static void list(FILE *o) {
  int i;
  for (i = 0; commands[i].name; i++) {
    fprintf(o, "%s\t\t%s\n", commands[i].name, commands[i].descr);
  }
}

static int cmd_parse_option(int argc, const char **argv, struct cmd_opt *opt) {
  return 0;
}

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

int main(int argc, const char **argv) {
  if (argc < 2)
    help_exit(EXIT_SUCCESS, stderr);

  int i;
  struct cmd_opt cmd_opts[32];
  int cmd_opt_c = 0;

  for (i = 2; i < argc; i++) {

    if (argv[i][0] != '-' || (strlen(argv[i]) == 1 && argv[i][0] == '-'))
      break;

    if (!cmd_parse_option(argc - i, &(argv[i]), &cmd_opts[cmd_opt_c]))
      help_exit_fmt(EXIT_FAILURE, stderr, "Invalid option %s.\n", argv[i]);

    cmd_opt_c++;
  }

  FILE *out = stdout;
  struct cmd_opt *output = opt_find(cmd_opt_c, cmd_opts, "o", "output");

  if (output) {
    char buf[256];
    strncpy(buf, output->value, output->value_c);

    out = fopen(buf, "w");
    if (!out)
      error(EXIT_FAILURE, "output file error: %s\n", strerror(errno));
  }

  struct cmd_in cmd_in;
  i = cmd_opt_c + 2;

  if (argc - i < 1) {
    cmd_in.kind = CMD_IN_INPLACE;
    cmd_in.inplace_len = 0;
    cmd_in.inplace = (void *)0;
  } else if (argc - i == 1) {
    cmd_in.kind = CMD_IN_FILE;
    if (strcmp(argv[i], "-") == 0)
      // stdin
      cmd_in.file = stdin;
    else if (iap_ip_parse(argv[i], strlen(argv[i]), NULL)) {
      cmd_in.inplace_len = 1;
      cmd_in.inplace = &argv[i];
      cmd_in.kind = CMD_IN_INPLACE;
    } else {
      cmd_in.file = fopen(argv[i], "r");
      if (!cmd_in.file)
        error(EXIT_FAILURE, "cannot open input file %s: %s.", argv[i],
              strerror(errno));
    }
  } else {
    cmd_in.kind = CMD_IN_INPLACE;
    cmd_in.inplace_len = 0;

    cmd_in.inplace = &argv[i];
    for (; i < argc; i++) {
      if (!iap_ip_parse(argv[i], strlen(argv[i]), NULL))
        error(EXIT_FAILURE, "invalid address: %s.", argv[i]);

      cmd_in.inplace_len++;
    }
  }

  struct command *cmd = (void *)0;
  for (i = 0; commands[i].name; i++) {
    if (strcmp(commands[i].name, argv[1]) == 0) {
      cmd = &commands[i];
    }
  }

  int rc = EXIT_SUCCESS;

  struct cmd_data data = {
      .cmd_opt_c = cmd_opt_c, .cmd_opts = cmd_opts, .in = &cmd_in, .out = out};

  if (cmd) {
    rc = cmd->proc(&data);
  } else {
    help_exit_fmt(EXIT_FAILURE, stdout, "undefined command: %s.", argv[1]);
  }

  return rc;
}