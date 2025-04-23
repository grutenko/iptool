#include "cmd.h"
#include "core.h"

#include <errno.h>
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

void parse_ips(int argc, char **argv, iap_t **root) {
  // Implementation of the parse_ips function
  if (argc == 0)
    return;

  FILE *in = stdin;
  iap_t a = {0}, b = {0};
  char buffer[256] = {0}, *buffer_p = buffer;
  int c, i;

  if (argc == 1) {
    strncpy(buffer, argv[0], sizeof(buffer) - 1);

    if (strlen(argv[0]) > 1 && argv[0][0] == '@') {
      // file
      in = fopen(argv[0] + 1, "r");
      if (!in)
        goto _io_error;
    } else if (strcmp(argv[0], "-") == 0) {
      // stdin
      ;
    } else if (iap_aton(argv[0], strlen(argv[0]), NULL) > 0) {
      strncpy(buffer, argv[0], sizeof(buffer) - 1);
      // ip address
      iap_aton(argv[0], strlen(argv[0]), &a);

      if (iap_insert(root, &a) == 0)
        goto _mem_failure;

      return;
    } else if (iap_range_aton(argv[0], strlen(argv[0]), NULL, NULL) > 0) {
      // ip range
      iap_range_aton(argv[0], strlen(argv[0]), &a, &b);

      if (iap_range_insert(&a, &b, root) == 0)
        goto _mem_failure;

      return;
    } else {
      goto _parse_error;
    }

    c = fgetc(in);
    while (c != EOF) {
      if (strchr("0123456789./-", c)) {
        if (buffer_p - buffer >= sizeof(buffer) - 1)
          goto _parse_error;

        *buffer_p++ = c;
      } else if (strchr(" ,\t\r\n", c)) {
        *buffer_p = '\0';

        if (iap_aton(buffer, buffer_p - buffer, &a) > 0) {
          if (iap_insert(root, &a) == 0)
            goto _mem_failure;
        } else if (iap_range_aton(buffer, buffer_p - buffer, &a, &b) > 0) {
          if (iap_range_insert(&a, &b, root) == 0)
            goto _mem_failure;
        } else {
          goto _parse_error;
        }

        buffer_p = buffer;
      } else {
        goto _invalid_char;
      }

      c = fgetc(in);
    }

    if (in != stdin) {
      fclose(in);
    }
  } else {
    for (i = 0; i < argc; i++) {
      strncpy(buffer, argv[i], sizeof(buffer) - 1);
      // ip range
      if (iap_range_aton(argv[i], strlen(argv[i]), &a, &b) > 0) {

        if (iap_range_insert(&a, &b, root) == 0)
          goto _mem_failure;

      } else if (iap_aton(argv[i], strlen(argv[i]), &a) > 0) {

        if (iap_insert(root, &a) == 0)
          goto _mem_failure;

      } else {
        goto _parse_error;
      }
    }
  }

  return;
_io_error:
  fprintf(stderr, "Error: Failed to open file '%s'. %s\n", argv[0] + 1,
          strerror(errno));
  iap_free(root);
  exit(EXIT_FAILURE);
_parse_error:
  fprintf(stderr, "Error: Failed to parse input: %s\n", buffer);
  iap_free(root);
  exit(EXIT_FAILURE);
_mem_failure:
  fprintf(stderr, "Error: Failed to allocate memory\n");
  iap_free(root);
  exit(EXIT_FAILURE);
_invalid_char:
  fprintf(stderr, "Error: Invalid character '%c' in input\n", c);
  iap_free(root);
  exit(EXIT_FAILURE);
}
