#include "cmd.h"
#include "core.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clang-format off

struct cmd_struct commands[] = {
    {"invert", "Invert list of subnets.", cmd_invert, cmd_invert_help},
    {"filter", "filter list of addresses by subnet.", cmd_filter, cmd_filter_help},
    {"inflate", "inflate (expand list of subnets)", cmd_inflate, cmd_inflate_help},
    {"deflate", "deflate (find subnets)", cmd_deflate, cmd_deflate_help},
    {"help", "Show help message", cmd_help, NULL},
    {"list", "List all commands", cmd_list, NULL},
    {NULL}};

// clang-format on

struct cmd_struct *find_cmd(const char *name) {
  for (size_t i = 0; commands[i].name; i++) {
    if (strcmp(commands[i].name, name) == 0)
      return &commands[i];
  }
  return NULL;
}

struct cmd_struct *list_cmd() { return commands; }

static void parse_fail(iap_t **root, const char *msg, ...) {
  va_list va;
  va_start(va, msg);
  fwrite("Error: ", strlen("Error: "), 1, stderr);
  vfprintf(stderr, msg, va);
  va_end(va);
  iap_free(root);
  putc('\n', stderr);
  exit(EXIT_FAILURE);
}

static void parse_token(iap_t **root, const char *str) {
  iap_t a = {0}, b = {0};
  if (iap_aton(str, strlen(str), &a) > 0) {
    if (iap_insert(root, &a) == 0)
      parse_fail(root, "failed to allocate memory");
  } else if (iap_range_aton(str, strlen(str), &a, &b) > 0) {
    if (iap_range_insert(&a, &b, root) == 0)
      parse_fail(root, "failed to allocate memory");
  } else {
    parse_fail(root, "failed to parse input: %s", str);
  }
}

void parse_ips(int argc, char **argv, iap_t **root) {
  // Implementation of the parse_ips function
  if (argc == 0)
    return;

  FILE *in = stdin;
  char buffer[256] = {0}, *buffer_p = buffer;
  int c, i;
  const char *ipchars = "0123456789./-";
  const char *delimiters = " ,\t\r\n";

  if (argc == 1) {
    strncpy(buffer, argv[0], sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    if (argv[0][0] == '@' && strlen(argv[0]) > 1) {
      // file
      in = fopen(argv[0] + 1, "r");
      if (!in)
        parse_fail(root, "failed to open file '%s': %s", buffer,
                   strerror(errno));
    } else if (strcmp(argv[0], "-") == 0) {
      // stdin
      ;
    } else if (iap_aton(argv[0], strlen(argv[0]), NULL) > 0 ||
               iap_range_aton(argv[0], strlen(argv[0]), NULL, NULL) > 0) {
      parse_token(root, buffer);
      return;
    } else {
      parse_fail(root, "failed to parse input: %s\n", buffer);
    }
    c = fgetc(in);
    while (c != EOF) {
      if (strchr(ipchars, c)) {
        if (buffer_p - buffer >= sizeof(buffer) - 1)
          parse_fail(root, "failed to parse input: %s", buffer);
        *buffer_p++ = c;
        *buffer_p = '\0';
      } else if (strchr(delimiters, c)) {
        parse_token(root, buffer);
        buffer_p = buffer;
      } else {
        parse_fail(root, "invalid character: %c", c);
      }
      c = fgetc(in);
    }
    if (in != stdin) {
      fclose(in);
    }
  } else {
    for (i = 0; i < argc; i++) {
      strncpy(buffer, argv[i], sizeof(buffer) - 1);
      buffer[sizeof(buffer) - 1] = '\0';
      // ip range
      parse_token(root, buffer);
    }
  }
}
