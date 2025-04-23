#include "iap.h"
#include "cmd.h"

#include <stdio.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Error: Invalid count of arguments.\n\n" SHORT_USAGE);
    return 1;
  }

  struct cmd_struct *cmd = find_cmd(argv[1]);
  int rc;

  if (cmd) {
    if(argc == 2) {
      rc = cmd->proc(0, NULL);
    } else {
      rc = cmd->proc(argc - 2, &argv[2]);
    }
  } else {
    fprintf(stderr, "Error: Invalid command.\n\n" SHORT_USAGE);
    return 1;
  }

  return rc;
}
