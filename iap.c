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

static int cmd_parse_option(int argc, const char **argv, struct cmd_opt *opt) {
  return 0;
}

static struct cmd_opt *opt_find(struct cmd_opt *opt, const char *_short,
                                const char *_long) {
  int i = 0;
  while (opt[i].key) {
    if ((_short && opt[i].key_c == strlen(_short) &&
         strncmp(opt[i].key, _short, opt[i].key_c) == 0) ||
        (_long && opt[i].key_c == strlen(_long) &&
         strncmp(opt[i].key, _long, opt[i].key_c) == 0))
      return &opt[i];
    i++;
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
  struct cmd_opt *output = opt_find(cmd_opts, "o", "output");

  if (output) {
    char buf[256];
    strncpy(buf, output->value, output->value_c);

    out = fopen(buf, "w");
    if (!out)
      error(EXIT_FAILURE, "output file error: %s\n", strerror(errno));
  }

  i = cmd_opt_c + 2;

  iap_t *r = {0};
  iap_t a = {0};

  if (argc - i < 1) {
    // ...
  } else if ((argc - i == 1 && iap_aton(argv[i], strlen(argv[i]), NULL)) ||
             argc - i > 1) {
    for (; i < argc; i++) {
      if (!iap_aton(argv[i], strlen(argv[i]), &a))
        error(EXIT_FAILURE, "invalid address: %s", argv[i]);

      if (!iap_insert(&r, &a))
        error(EXIT_FAILURE, "memory allocation error.");
    }
  } else if (argc - i == 1) {
    FILE *in;
    // file
    if (strcmp(argv[i], "-") == 0) {
      in = stdin;
    } else {
      in = fopen(argv[i], "r");
      if (!in)
        error(EXIT_FAILURE, "cannot open input file: %s", strerror(errno));
    }

    int c;
    char buffer[IAP_BEST_LEN + 1], *buffer_p = buffer;
    c = getc(in);
    while (c != EOF) {
      if ((c >= '0' && c <= '9') || c == '.' || c == '/') {
        if (buffer_p - buffer >= IAP_BEST_LEN)
          error(EXIT_FAILURE, "invalid address: %s", buffer);

        *buffer_p++ = c;
        *buffer_p = '\0';
      } else if (c == ' ' || c == '\n' || c == ',') {
        if (buffer_p - buffer > 0) {
          if (!iap_aton(buffer, buffer_p - buffer, &a))
            error(EXIT_FAILURE, "invalid address: %s\n", buffer);

          if (!iap_insert(&r, &a))
            error(EXIT_FAILURE, "memory allocation error.");

          buffer_p = buffer;
        }
      } else {
        error(EXIT_FAILURE, "invalid character: %c\n", c);
      }
      c = getc(in);
    }

    fclose(in);
  } else {
    error(EXIT_FAILURE, "invalid address list.");
  }

  struct command *cmd = (void *)0;
  for (i = 0; commands[i].name; i++) {
    if (strcmp(commands[i].name, argv[1]) == 0) {
      cmd = &commands[i];
    }
  }

  int rc = EXIT_SUCCESS;

  if (cmd) {
    rc = cmd->proc(&r, out);
  } else {
    help_exit_fmt(EXIT_FAILURE, stdout, "undefined command: %s.", argv[1]);
  }
  iap_free(&r);

  return rc;
}
